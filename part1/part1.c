#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

// Include DE1 Specific Address Maps
#include "address_map_arm.h"
#include "fileio.h"
#include "ADXL345.h"

unsigned int * SYSMGRVirt;
unsigned int * I2C0Virt;


int Running = 1;

void SIGINTHandler(int Interrupt) {
  printf("\n[ctrl]+[c] Encountered: exiting.\n");
  Running = 0;
}


/* This program increments the contents of the red LED parallel port */
int main(void) {

  uint8_t DevID;
  int16_t MGPerLSB = 4;
  int16_t XYZ[3];
 
  int fd = -1; // used to open /dev/mem for access to physical addresses
  // Handle ctrl+c;
  signal(SIGINT, SIGINTHandler);


  // Create virtual memory access to the FPGA light-weight bridge
  if ((fd = open_physical(fd)) == -1)
    return -1;

  // Map the SYSMGR addresses into virtual space.
  if ((SYSMGRVirt = map_physical(fd, SYSMGR_BASE, SYSMGR_SPAN)) == NULL)
    return -1;  

  // Map the I2C0 address into virtual space.
  if ((I2C0Virt = map_physical(fd, I2C0_BASE, I2C0_SPAN)) == NULL)
    return -1;    

  // Configure Pin Muxing
  Pinmux_Config();

  // Initialize I2C0 Controller
  I2C0_Init();

  // 0xE5 is read from DEVID(0x00) if I2C is functioning correctly
  ADXL345_REG_READ(ADXL345_REG_DEVID, &DevID);

  if (DevID != 0xE5) {
    printf("Incorrect Device ID.\n");
    return -1;
  }

  MGPerLSB = ROUNDED_DIVISION(16*1000, 512);
  ADXL345_Init();
  ADXL345_Calibrate();

  while (Running) {
    if (ADXL345_WasActivityUpdated()) {
      ADXL345_XYZ_Read(XYZ);
      printf("X=%d mg, Y=%d mg, Z=%d mg\n", XYZ[0]*MGPerLSB, XYZ[1]*MGPerLSB, XYZ[2]*MGPerLSB);
    }
  }


  unmap_physical(SYSMGRVirt, SYSMGR_SPAN); // release the physical-memory mapping
  unmap_physical(I2C0Virt, I2C0_SPAN); // release the physical-memory mapping
  close_physical(fd);             // close /dev/mem

  return 0;
}
