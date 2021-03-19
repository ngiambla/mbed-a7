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

  int16_t X;
  int16_t Y;
  int16_t Z;
  int i;
  uint8_t InterruptStatus;
  int16_t ScaleFactor;
  char OutputString[50] = "-- ---- ---- ---- --";

  float AvgX = 0, AvgY = 0;

  // 1. Register the SIGINT handler.
  signal(SIGINT, IntHandler);
  // 2. Using the API from driverutils.h, open the driver(s)
  OpenDrivers();
  // 3. Re-Initialize the Accelerometer
  WriteTo(ACCEL, "init", 4);
  // 4. Calibrate the accelerometer.
  WriteTo(ACCEL, "calibrate", 9);

  // 5. Initialize the terminal to be "drawable"
  InitializeTerminal();

  while (Running) {
    // 6. Read from the accel driver.
    ReadFrom(ACCEL, AccelReadBuffer, ACCEL_READ_SIZE);

    ClearTerminal();

    // 7. Re-interpret the string from AccelReadBuffer via sscanf into
    // variables.
    if (sscanf(AccelReadBuffer, "%hhx %hd %hd %hd %hd", &InterruptStatus, &X,
               &Y, &Z, &ScaleFactor) < 0) {
      ErrorHandler("Could not determine accelerometer output.");
    }

    // 8. If the InterruptStatus indicates we have data, display the data on the
    // top-left of the screen (as a string)
    if (InterruptStatus & ACCEL_DATAREADY) {
      if (snprintf(OutputString, 50, "X=%4d Y=%4d Z=%4d (milli m/s^2)\n",
                   X * ScaleFactor, Y * ScaleFactor, Z * ScaleFactor) < 0) {
        printf("Error: snprintf was unsuccessful");
        // Terminate the string at pos 0.
        OutputString[0] = '\0';
      }


      // Now, take those coordinates, and fill-in the fields of the circle
      // struct (with respect to the center of the terminal) We use a smoothing
      // factor here by taking a moving average!
      AvgX = AvgX * 0.5 + X * (0.5);
      AvgY = AvgY * 0.5 + Y * (0.5);
      Main.X = (int)AvgX + (XRange >> 1);
      Main.Y = (int)AvgY + (YRange >> 1);
      // Set the radius to be 4.
      Main.R = 4;
      // The circle is indeed valid.
      Main.Valid = 1;
    }
    for (i = 0; i < strlen(OutputString) - 1; ++i)
      PlotChar(i + 1, 3, GREEN, OutputString[i]);

    // Plot the circle if the circle is valid.
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
