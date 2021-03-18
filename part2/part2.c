#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "driverutils.h"

volatile sig_atomic_t Running = 1;
struct timespec AnimationTime;

void IntHandler(int inter) { Running = 0; }

int main() {

  int16_t X;
  int16_t Y;
  int16_t Z;
  uint8_t InterruptStatus;
  int16_t ScaleFactor;

  // 1. Register the SIGINT handler.
  signal(SIGINT, IntHandler);
  // 2. Using the API from driverutils.h, open the driver(s)
  OpenDrivers();
  // 3. Continously probe the driver for any accelerometer changes.
  while (Running) {
    ReadFrom(ACCEL, AccelReadBuffer, ACCEL_READ_SIZE);
    if (sscanf(AccelReadBuffer, "%hhx %hd %hd %hd %hd", &InterruptStatus, &X,
               &Y, &Z, &ScaleFactor) < 0) {
      ErrorHandler("Could not determine accelerometer output.");
    }
    if (InterruptStatus & ACCEL_DATAREADY) {
      printf("X=%4d Y=%4d Z=%4d (milli m/s^2)\n", X * ScaleFactor,
             Y * ScaleFactor, Z * ScaleFactor);
    }
  }
  ReleaseDrivers();
  return 0;
}