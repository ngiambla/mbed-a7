# Assignment 7


There are 4 parts to this assignments, each implemented in a unique directoy.

* `part1/`
* `part2/`
* `part3/`
* `part4/`

Additionally, a kernel module which communicates with the ADXL345 device exists in: `accelmod`.
This kernel module will be used by `part{2,3,4}/`

# Part 1

A user level program interacts with the ADXL345 Accelerometer device.

Build part 1: `cd part1; make clean; make;`
To Use: `./part1.exe`
To Exit: `[ctrl]+c`

During execution, the terminal should display the X-Y-Z state of the acceleration in each of those directions.

# Prior to Part {2, 3, 4}

As noted earlier, Parts 2, 3 and 4 require the use of the kernel module which interacts
with the ADXL345 accelerometer.

Although the lab specifies to slowly "build" up the kernel module (at each part, the kernel module becomes more sophisticated):
I've decided to encompass all required components into 1 kernel module (removing unnecessary code bloat.)

As noted in `accelmod/accel.c`:

```c
// NOTE: For Parts II, III and IV, I've reused the same kernel module,
//       since you can encapsulate all behaviours required by having
//       the output:
//                  "RR XXXX YYYY ZZZZ SS"
//       Reasoning:
//       (1) when reading the the kernel (you can know if the data is ready or not) from "RR"
//           which is required in part II and expanded to be generalized in Part IV.
//           Therefore, I've kept this version which satisfies ALL of the requirements.
//       (2) Part III instructs us to have the driver writable (as to issue commands) which is
//           also is used in Part IV.
```

* To build the kernel module: `cd accelmod; make clean; make`
* To insert the kernel module: `insmod accel.ko` _Please_ insert the kernel module prior to executing Parts 2, 3 and 4.
* To remove the kernel module (no part requires the removal of the module): `rmmod accel`

# Part 2

A user level program interacts with the Accelerometer kernel module, `accel`, and should provide
the same output as compared to Part 1.

Build part 1: `cd part2; make clean; make;`
To Use: `./part2.exe`
To Exit: `[ctrl]+c`


# Part 3

A user level program interacts with the Accelerometer kernel module, `accel`, and should display the movement of the DE1-SoC
via probing the module for changings in acceleration in the X, Y and X planes.

The DE1-SoC is visualized as a red-circle, which is initially centered in the middle of a terminal.
As the DE1-SoC moves, the red-circle will track it's position on screen.

Additionally, the acceleration in the X, Y and Z planes are displayed as a string messaged in the top left corner.

Build part 1: `cd part3; make clean; make;`
To Use: `./part3.exe`
To Exit: `[ctrl]+c`


# Part 4

A user level program interacts with the Accelerometer kernel module, `accel`, and should display the movement of the DE1-SoC
via probing the module for changings in acceleration in the X, Y and X planes.

The DE1-SoC is visualized as a red-circle, which is initially centered in the middle of a terminal.
As the DE1-SoC moves, the red-circle will track it's position on screen.

Additionally, the acceleration in the X, Y and Z planes are displayed as a string messaged in the top left corner.

Lastly, the user level program is able to ask the kernel module if the DE1-SoC has been tapped (either as a single tap
or a double tap). If a single OR double tap occurs, a message will be displayed in the top left corner of the terminal
(below the X, Y, Z data string) and will clear after (roughly) 2 seconds.

Build part 1: `cd part4; make clean; make;`
To Use: `./part4.exe`
To Exit: `[ctrl]+c`


# Notes:

Feel free to experiment with the commands we can issue to accel driver:

```
init: re-initializes the ADXL345
device: prints on the Terminal (using printk) the ADXL345 device ID.
calibrate: calibrates the device.
format F G: sets the data format to fixed 10-bit resolution (F = 0), or full resolution (F = 1), with range G = +/- 2, 4, 8, or 16 g
rate R: sets the output data rate to R Hz:
        As we note in ADXL345_SetFreq:
        (1) if the user requested freq doesn't exit, we default
            to 12.5 hz.
        (2) When requesting for non-whole number sampling freqs
           (i.e., 12.5, 6.25, 3.125 1.563), the user must only specify
           the integer value of these: (12 == 12.5, 6 = 6.25, etc.)
        (3) We support the frequency range from 3200 hz t0 1.563 hz.

```

You can issue a command like from the terminal like so: `echo "init" > /dev/accel`.
You can also issue one from a user-level program using our `driverutils.h` API (e.g., `WriteTo(...)`)
