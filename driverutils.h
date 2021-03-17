#ifndef __DRIVER_UTILS_H__
#define __DRIVER_UTILS_H__

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Bit values in INT_ENABLE, INT_MAP, and INT_SOURCE are identical
   use these bit values to read or write any of these registers.        */
#define ACCEL_OVERRUN              0x01
#define ACCEL_WATERMARK            0x02
#define ACCEL_FREEFALL             0x04
#define ACCEL_INACTIVITY           0x08
#define ACCEL_ACTIVITY             0x10
#define ACCEL_DOUBLETAP            0x20
#define ACCEL_SINGLETAP            0x40
#define ACCEL_DATAREADY            0x80


// Define number of drivers
#define NUM_DRIVERS 1

// Define Buffers for the two drivers
// as well as their size.
#define ACCEL_WRITE_SIZE 40
#define ACCEL_READ_SIZE 40

char AccelWriteBuffer[ACCEL_WRITE_SIZE];
char AccelReadBuffer[ACCEL_READ_SIZE];


// Define Drivers as Integer references.
#define ACCEL 0

// Define a DriverRef struct to simplify our
// development process.
struct DriverRef {
  char Path[15];
  int RWP;
  int FD;
};

struct DriverRef Drivers[NUM_DRIVERS] = {
    {.Path = "/dev/accel", .RWP = O_RDWR, .FD = -1}
};

// Using a Macro to get a Driver's Open File Desc.
#define GetFD(x) (Drivers[(x)].FD)
#define IsRDONLY(x) (Drivers[(x)].RWP == O_RDONLY)


// Loop through our drivers, and close all file
// descriptors that are open.
void ReleaseDrivers() {
  int i;
  for (i = 0; i < NUM_DRIVERS; ++i) {
  	if (GetFD(i) != -1)
  		close(GetFD(i));
  }
}

void ErrorHandler(char * Message) {
  ReleaseDrivers();
  fprintf(stderr, "Error Encountered: %s\nExiting %s\n", Message, strerror(errno));
  exit(-1);
}

void OpenDrivers() {
  int i;
  for (i = 0; i < NUM_DRIVERS; ++i) {
    if ((Drivers[i].FD = open(Drivers[i].Path, Drivers[i].RWP)) == -1) {
      ErrorHandler("Failed to open driver.");
    }
  }	
}



void ReadFrom(int DevId, char *Buffer, int BufSize) {
  int BytesRead = 0;
  int ReadStatus;
  while ((ReadStatus = read(GetFD(DevId), Buffer, BufSize)) != 0)
    BytesRead += ReadStatus; // read the driver until EOF

  if (ReadStatus < 0) {
    Buffer[0] = '\0';
    ErrorHandler("Read was unsuccessful.");
  }

  Buffer[BytesRead] = '\0'; // NULL terminate

  // Recall, We've implemented the lseek function for
  // read-only drivers (i.e., SW and KEYs)
  if (IsRDONLY(DevId))
    lseek(GetFD(DevId), 0, SEEK_SET);
}

void WriteTo(int DevId, char *Buffer, int BufSize) {
  if (write(GetFD(DevId), Buffer, BufSize) < 0) {
    ErrorHandler("Write was unsuccessful.");
  }
}

// Using strtoumax, convert a string to a uint.
// If successful, set Safe to be 1 and return the
// mapped value.
//
// Otherwise, set Safe to be 0 (indicating the conversion was
// not successful) and return 0.
uint32_t StringToUint(char *DriverMsg, uint8_t *Safe) {
  uint32_t IntegerValue = strtoumax(DriverMsg, NULL, 10);
  if (IntegerValue == UINTMAX_MAX && errno == ERANGE) {
    *Safe = 0;
    return 0;
  }
  *Safe = 1;
  return IntegerValue;
}


#endif