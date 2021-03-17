#include <asm/io.h>          // for mmap
#include <linux/fs.h>        // struct file, struct file_operations
#include <linux/init.h>      // for __init, see code
#include <linux/interrupt.h> // for interrupt handling
#include <linux/kernel.h>
#include <linux/miscdevice.h> // for misc_device_register and struct miscdev
#include <linux/module.h>     // for module init and exit macros
#include <linux/time.h>
#include <linux/uaccess.h> // for copy_to_user, see code

#include "../address_map_arm.h"
#include "ADXL345.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicholas Giamblanco");
MODULE_DESCRIPTION("ADXL345 Accelerometer Device Driver");

#define SUCCESS 0

// Defines for Registered State.
#define NOT_REGISTERED 0
#define REGISTERED 1

// Constant String for Accelerometer driver.
#define ACCEL_DEV_NAME "accel"

volatile unsigned int * SYSMGRVirt;
volatile unsigned int * I2C0Virt;

static int16_t MGPerLSB;
static uint8_t DevID;


#define ACCEL_READ_BUF_SIZE 21                   //RR XXXX YYYY ZZZZ SS
static char ACCEL_READ_BUF[ACCEL_READ_BUF_SIZE] = "-- No Data Ready. --";

#define ACCEL_WRITE_BUF_SIZE 40
static char ACCEL_WRITE_BUF[ACCEL_WRITE_BUF_SIZE] = {'\0'};

// Declare the methods the video device driver will require.
// NOTE: we only need to read from the driver to understand the
//       commands accepted by this driver.
static int     AccelDevOpen(struct inode *, struct file *);
static int     AccelDevRelease(struct inode *, struct file *);
static ssize_t AccelDevRead(struct file *, char *, size_t, loff_t *);
static ssize_t AccelDevWrite(struct file *, const char *, size_t, loff_t *);

// Define the File Operations for /dev/accel
static struct file_operations AccelDevFops = {.owner = THIS_MODULE,
                                              .read =    AccelDevRead,
                                              .write =   AccelDevWrite,
                                              .open =    AccelDevOpen,
                                              .release = AccelDevRelease};

// Setup Miscellaneous Dev Struct
// We need to set the permissions
// for the driver file:
//
//  User | Group | Other
// ------+-------+------
// R W X | R W X | R W X
// ------+-------+------
// 1 1 0 | 1 1 0 | 1 1 0
//
// Therefore .mode is 0666
static struct miscdevice AccelDev = {.minor = MISC_DYNAMIC_MINOR,
                                     .name = ACCEL_DEV_NAME,
                                     .fops = &AccelDevFops,
                                     .mode = 0666};

static int AccelDevRegistered = NOT_REGISTERED;

void AccelDataToStr(void) {
  int i;
  int16_t XYZ[3];

  char InterruptStatusStr[5] ={'\0'};
  char AccelDataStr[15] = {'\0'};
  char ScaleStr[3] = {'\0'};
  char AccelReadBufTemp[23] = {'\0'};
  
  uint8_t InterruptFlags = ADXL345_WhichInterrupts();

  strcpy(AccelReadBufTemp, ACCEL_READ_BUF);

  printk(KERN_INFO "0. %s %s\n", ACCEL_READ_BUF, AccelReadBufTemp);
  
  // First, Read in the interrupt register into the string.
  if (snprintf(InterruptStatusStr, 3, "%02x\n", InterruptFlags) < 0) {
    printk(KERN_ERR "Error [%s]: snprintf was unsuccessful", ACCEL_DEV_NAME);
    return;
  }
  // Copy this into the AccelReadBufTemp
  for(i = 0; i < 2; ++i) {
    AccelReadBufTemp[i] = InterruptStatusStr[i];
  }

  printk(KERN_INFO "1. %s %s\n", ACCEL_READ_BUF, AccelReadBufTemp);

  // If Data is ready, update the portion of the string
  // with fresh data.
  if (InterruptFlags & XL345_DATAREADY) {
    ADXL345_XYZ_Read(XYZ);
    if (snprintf(AccelDataStr, 15, "%04d %04d %04d\n", XYZ[0], XYZ[1], XYZ[2]) < 0) {
      printk(KERN_ERR "Error [%s]: snprintf was unsuccessful", ACCEL_DEV_NAME);
      return;
    }

    for(i = 3; i < 17; ++i) {
      AccelReadBufTemp[i] = AccelDataStr[i-3];
    }
  }
  printk(KERN_INFO "2. %s %s\n", ACCEL_READ_BUF, AccelReadBufTemp);


  if (snprintf(ScaleStr, 3, "%02d\n", MGPerLSB) < 0) {
    printk(KERN_ERR "Error [%s]: snprintf was unsuccessful", ACCEL_DEV_NAME);
    return;
  }
  for(i = 18; i < 20; ++i) {
    AccelReadBufTemp[i] = ScaleStr[i-18];
  }
  printk(KERN_INFO "3. %s %s\n", ACCEL_READ_BUF, AccelReadBufTemp);


  AccelReadBufTemp[20] = '\n';
  AccelReadBufTemp[21] = '\0';
  strcpy(ACCEL_READ_BUF, AccelReadBufTemp);

  printk(KERN_INFO "4. %s %s\n", ACCEL_READ_BUF, AccelReadBufTemp);


}


void InterpCommand(char *Command) {
  uint8_t Resolution;
  uint8_t Gravity;
  uint16_t Rate;


  if (strncmp(Command, "init", 4) == 0) {
    // init: re-initializes the ADXL345
    MGPerLSB = ROUNDED_DIVISION(16*1000, 512);
    ADXL345_Init();
    return;
  }

  if(strncmp(Command, "device", 6) == 0) {
    // device: prints on the Terminal (using printk) the ADXL345 device ID.
    printk(KERN_INFO "Accelerometer Device ID: %08x\n", DevID);
    return;
  }

  if(strncmp(Command, "calibrate", 9) == 0) {
    // calibrate: calibrates the device.
    ADXL345_Calibrate();
    return;
  }

  if(strncmp(Command, "format", 6) == 0) {
    // format F G: sets the data format to fixed 10-bit resolution (F = 0), or full resolution (F = 1), with
    //   range G = +/- 2, 4, 8, or 16 g    
    if (sscanf(Command + 6, "%*[^0123456789]%hhd %hhd", &Resolution, &Gravity) < 2)
      return;
    if(Resolution > 1) return;
    ADXL345_SetG(Resolution, Gravity, &MGPerLSB);
    return;
  }

  if(strncmp(Command, "rate", 4) == 0) {
    // rate R: sets the output data rate to R Hz. Your code should support a few examples of data rates, such
    // as 25 Hz, 12.5 Hz, and so on.
    if (sscanf(Command + 6, "%*[^0123456789]%hd", &Rate) < 1)
      return;
    ADXL345_SetFreq(Rate);
    return;
  }

}

static int __init init_accel(void) {
  // This is an early exit strategy for initializtion:
  int AccelRegisterStatus;

  // 1. Register the Accel Device Driver.
  AccelRegisterStatus = misc_register(&AccelDev);
  if (AccelRegisterStatus < 0) {
    // If the status returned by misc_register is less than 0,
    // early exit the init... (something has gone wrong).
    printk(KERN_ERR "/dev/%s: misc_register() failed\n", ACCEL_DEV_NAME);
    return AccelRegisterStatus;
  }
  // 2. Log that we've registered the Accelerometer Device driver
  printk(KERN_INFO "/dev/%s driver registered\n", ACCEL_DEV_NAME);
  AccelDevRegistered = REGISTERED;

  // 3. Map SYSMGR into virtual mem.
  SYSMGRVirt = ioremap_nocache(SYSMGR_BASE, SYSMGR_SPAN);
  if (!SYSMGRVirt) {
    printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");
    AccelDevRegistered = NOT_REGISTERED;
    misc_deregister(&AccelDev);
  }

  // 4. Map I2C0 into virtual mem.
  I2C0Virt = ioremap_nocache(I2C0_BASE, I2C0_SPAN);
  if (!I2C0Virt) {
    printk(KERN_ERR "Error: ioremap_nocache returned NULL\n");
    iounmap(SYSMGRVirt);
    AccelDevRegistered = NOT_REGISTERED;
    misc_deregister(&AccelDev);
  }


  // Configure Pin Muxing
  Pinmux_Config();

  // Initialize I2C0 Controller
  I2C0_Init();

  // 0xE5 is read from DEVID(0x00) if I2C is functioning correctly
  ADXL345_REG_READ(ADXL345_REG_DEVID, &DevID);

  if (DevID != 0xE5) {
    printk(KERN_ERR "Accelerometer ID is incorrect\n");
    iounmap(SYSMGRVirt);
    iounmap(I2C0Virt);
    AccelDevRegistered = NOT_REGISTERED;
    misc_deregister(&AccelDev);
  }

  MGPerLSB = ROUNDED_DIVISION(16*1000, 512);

  ADXL345_Init();
  ADXL345_Calibrate();

  return AccelRegisterStatus;
}

static void __exit stop_accel(void) {
  if (AccelDevRegistered) {
    iounmap(SYSMGRVirt);
    iounmap(I2C0Virt);
    // Proceed with de-registering the character driver
    misc_deregister(&AccelDev);
    printk(KERN_INFO "/dev/%s driver de-registered\n", ACCEL_DEV_NAME);
  }
}

/* Called when a process opens /dev/accel */
static int AccelDevOpen(struct inode *inode, struct file *file) {
  return SUCCESS;
}

/* Called when a process closes /dev/accel */
static int AccelDevRelease(struct inode *inode, struct file *file) { return 0; }


static ssize_t AccelDevRead(struct file *FilP, char *Buffer, size_t Length,
                            loff_t *Offset) {

  // Bytes to Sendout.
  size_t BytesToSend;

  if(!(*Offset)) {
    AccelDataToStr();
  }

  // 1. Determine How many bytes to Send:
  //    (a) Find How many Outstanding bytes there are
  BytesToSend = strlen(ACCEL_READ_BUF) - (*Offset);
  //    (b) Send the Maximum number of bytes user space can handle.
  BytesToSend = BytesToSend > Length ? Length : BytesToSend;
  // 3. Send out bytes to user space.
  if (BytesToSend > 0) {
    if (copy_to_user(Buffer, &ACCEL_READ_BUF[*Offset], BytesToSend) != 0)
      printk(KERN_ERR "Error [%s]: copy_to_user unsuccessful", ACCEL_DEV_NAME);
    // Update the File Ptr's Offset to reflect where to read from next read.
    *Offset += BytesToSend;
  }
  // 3. If the number of bytes is 0, reset offset.
  //    This allows the next read to "read" from the beginning of the file.
  if (BytesToSend == 0)
    *Offset = 0;
  return BytesToSend;
}

static ssize_t AccelDevWrite(struct file *FilP, const char *Buffer,
                             size_t Length, loff_t *Offset) {
  // 1. Store the Length of the Message that user has written to us.
  size_t BytesRead = Length;

  // 2. Can we copy the entire message at once (is our buffer large enough)
  if (BytesRead > ACCEL_WRITE_BUF_SIZE - 1)
    BytesRead = ACCEL_WRITE_BUF_SIZE - 1;

  // 3. Copy the data from user space here, to our buffer.
  if (copy_from_user(ACCEL_WRITE_BUF, Buffer, BytesRead)) {
    printk(KERN_ERR "Error [%s]: Couldn't copy all bytes via copy_from_user",
           ACCEL_DEV_NAME);
    return -EFAULT;
  }

  ACCEL_WRITE_BUF[BytesRead] = '\0'; // NULL terminate
  // Process the command.
  InterpCommand(ACCEL_WRITE_BUF);
  // Notes:
  // 1. We do NOT update *offset (although, it could be done)
  // 2. We return Length (to fake-out the write operation). That is
  //    By returning Length, We only read the first min(Length, CMD_MAX_SIZE)
  //    bytes
  return Length;
}

module_init(init_accel);
module_exit(stop_accel);