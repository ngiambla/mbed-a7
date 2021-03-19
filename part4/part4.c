#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "driverutils.h"
#include "plotutils.h"

volatile sig_atomic_t Running = 1;
struct timespec AnimationTime;

void IntHandler(int inter) { Running = 0; }

int main() {

  clock_t SingleStartTime;
  clock_t DoubleStartTime;
  int16_t X;
  int16_t Y;
  int16_t Z;
  int i;
  uint8_t InterruptStatus;
  int16_t ScaleFactor;
  char OutputString[50] = {'\0'};
  char SingleTapEvent[] = "Single Tap!";
  char DoubleTapEvent[] = "Double Tap!";

  float AvgX = 0, AvgY = 0;

  // 1. Register the SIGINT handler.
  signal(SIGINT, IntHandler);
  // 2. Using the API from driverutils.h, open the driver(s)
  OpenDrivers();
  // 3. Re-Initialize the Accelerometer
  WriteTo(ACCEL, "init", 4);
  // 4. Calibrate the accelerometer.
  WriteTo(ACCEL, "calibrate", 9);

  InitializeTerminal();

  while (Running) {
    // 5. Read from /dev/accel, find out if we've received data.
    ReadFrom(ACCEL, AccelReadBuffer, ACCEL_READ_SIZE);


    ClearTerminal();


    // Re-interpret the string from AccelReadBuffer via sscanf into variables.
    if (sscanf(AccelReadBuffer, "%hhx %hd %hd %hd %hd", &InterruptStatus, &X,
               &Y, &Z, &ScaleFactor) < 0) {
      ErrorHandler("Could not determine accelerometer output.");
    }

    // If the InterruptStatus indicates we have data, display the data on the
    // top-left of the screen (as a string)
    if (InterruptStatus & ACCEL_DATAREADY) {
      if (snprintf(OutputString, 50, "X=%4d Y=%4d Z=%4d (milli m/s^2)\n",
                   X * ScaleFactor, Y * ScaleFactor, Z * ScaleFactor) < 0) {
        printf("Error: snprintf was unsuccessful");
        // Terminate the string at pos 0.
        OutputString[0] = '\0';
      }


      AvgX = AvgX * 0.5 + X * (0.5);
      AvgY = AvgY * 0.5 + Y * (0.5);
      Main.X = (int)AvgX + (XRange >> 1);
      Main.Y = (int)AvgY + (YRange >> 1);
      Main.R = 3;
      Main.Valid = 1;
    }
    // Also ask InterruptStatus if a SINGLETAP or DOUBLETAP event has been
    // captured. (e.g., the interrupts should be high) If a SINGLETAP or
    // DOUBLETAP event has occured, display: "Single Tap!" or "Double Tap!"
    // below the XYZ string.
    if (InterruptStatus & ACCEL_SINGLETAP) {
      SingleStartTime = clock();
    }

    if (InterruptStatus & ACCEL_DOUBLETAP) {
      DoubleStartTime = clock();
    }
    for (i = 0; i < strlen(OutputString) - 1; ++i)
      PlotChar(i + 1, 1, GREEN, OutputString[i]);
    
    if(((clock() - SingleStartTime) / CLOCKS_PER_SEC) < 2.0)
      for (i = 0; i < 11; ++i)
        PlotChar(i + 1, 3, YELLOW, SingleTapEvent[i]); 
    if(((clock() - DoubleStartTime) / CLOCKS_PER_SEC) < 2.0)
      for (i = 0; i < 11; ++i)
        PlotChar(i + 1, 4, MAGENTA, DoubleTapEvent[i]);

    if (Main.Valid)
      PlotCircle(Main.X, Main.Y, Main.R, RED);

    nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);

  }
  ResetTerminal();
  // Flush all in buffer to stdout.
  fflush(stdout);
  // Release all drivers.
  ReleaseDrivers();
  return 0;
}