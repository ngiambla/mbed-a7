#include <signal.h>
#include <stdio.h>
#include <time.h>


#include "driverutils.h"


volatile sig_atomic_t Running = 1;
struct timespec AnimationTime;

void IntHandler(int inter) { Running = 0; }

int main() {

  int X;
  int Y;
  int Z;
  int InterruptStatus;
  int ScaleFactor;

  // 1. Register the SIGINT handler.
  signal(SIGINT, IntHandler);
  // 2. Using the API from driverutils.h, open the driver(s)
  OpenDrivers();
  // 3. Continously probe the driver for any accelerometer changes.
  while (Running) {
    ReadFrom(ACCEL, AccelReadBuffer, AccelReadSize);
    if (sscanf(AccelReadBuffer, "%d %d %d %d %d", &InterruptStatus, &X, &Y, &Z, &ScaleFactor) < 0) {
      ErrorHandler("Could not determine screen dimensions.");
    }
    if (InterruptStatus & ACCEL_DATAREADY) {
      printf("X=%4d Y=%4d Z=%4d (milli m/s^2)\n", X*ScaleFactor, Y*ScaleFactor, Z*ScaleFactor);
    }
  }
  ReleaseDrivers();
  return 0;
}