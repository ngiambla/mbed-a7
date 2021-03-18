#ifndef __PLOTUTILS_H__
#define __PLOTUTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

// VT100 Color Codes
#define BLACK 30
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define MAGENTA 35
#define CYAN 36
#define LT_GRAY 37
#define DK_GRAY 90
#define LT_RED 91
#define LT_GREEN 92
#define LT_YELLOW 93
#define LT_BLUE 94
#define LT_MAGENTA 95
#define LT_CYAN 96
#define WHITE 97

#define NUM_LETTERS 25


// This struct will represent a circle to plot on the screen with a particular radius.
struct Circle {
  int X;     // X Coord
  int Y;     // Y Coord
  int R;     // Radius
  int Color; // Color for ASCII disp
  int Valid; // We are not using dynmem, and so we have a globally allocated var, which cannot be "null"
             // This flag will be used to indicate if the Circle is valid.
};

// Create two static global variables (i.e., storage local
// to this scope) which will represent the maximum X and Y range
// of the terminal (the terminal size)
static int XRange = 4;
static int YRange = 4;


// The global variable for the circle to draw on screen (representing the position of the accelerometer)
struct Circle Main;


/* BEGIN VT100 Helper Functions */

// Set Color of Text.
void SetTextColor(int Color) {
  printf("\e[%dm", Color);
  fflush(stdout);
}


// Resets terminal to initial state
void ResetTerminal() {
  printf("\ec");
  fflush(stdout);
}

// Sets the cursor at the X (i.e., col) and Y (i.e., row) of the
// terminal
void SetCursorAt(int X, int Y) {
  printf("\e[%d;%dH", Y, X);
  fflush(stdout);
}

// Provided with a coordinate (X,Y), a Color (e.g., color code for blue) 
// and Dispchar (e.g., '@'), plot it on the terminal.
void PlotChar(int X, int Y, char Color, char Dispchar) {
  printf("\e[%2dm\e[%d;%dH%c\e[0m", Color, Y, X, Dispchar);
  fflush(stdout);
}

// Clear the screen
void ClearTerminal() {
  printf("\e[2J");
  fflush(stdout);
}

// Hide the cursor
void HideCursor() {
  printf("\e[?25l");
  fflush(stdout);
}

// Show the cursor
void ShowCursor() {
  printf("\e[?25h");
  fflush(stdout);
}

// Here, we can probe the Kernel for the current terminal
// size.
// From https://man7.org/linux/man-pages/man4/tty_ioctl.4.html:
//
// ioctl_tty - ioctls for terminals and serial lines
//
// Get and set window size
//      Window sizes are kept in the kernel, but not used by the kernel
//      (except in the case of virtual consoles, where the kernel will
//      update the window size when the size of the virtual console
//      changes, for example, by loading a new font).
//
//      The following constants and structure are defined in
//      <sys/ioctl.h>.
//
//      TIOCGWINSZ     struct winsize *argp
//             Get window size.
void GetTerminalSize() {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  XRange = w.ws_col;
  YRange = w.ws_row;
}

// The terminal will be cleared, and the cursor
// will be hidden.
void InitializeTerminal() {
  Main.Valid = 0;
  HideCursor();
  ClearTerminal();
  GetTerminalSize();
}

/* END VT100 Helper Functions */



// Follows Bresenham's Algorithm (this ver. is valid for ALL quadrants.)
void GeneralizedPlotLine(int X0, int Y0, int X1, int Y1, int Color, char Sym) {
  // Absolute change in X
  int dX = abs(X1 - X0);
  // Change in slope for X
  int sX = X0 < X1 ? 1 : -1;

  // Absolute change in Y
  int dY = -abs(Y1 - Y0);
  // Change in slop for Y
  int sY = Y0 < Y1 ? 1 : -1;
  // Error
  int E = dX + dY;
  int DoubleE;

  for (;;) {
    PlotChar(X0, Y0, Color, Sym);
    DoubleE = E << 1;
    /* Check if the Error between X and Y is > dX */
    if (DoubleE >= dY) {
      if (X0 == X1)
        break;
      E += dY;
      X0 += sX;
    }
    /* Check if the Error between X and Y is > dY */
    if (DoubleE <= dX) {
      if (Y0 == Y1)
        break;
      E += dX;
      Y0 += sY;
    }
  }
}


// NEW: Circle Drawing Utilities:
// We present two new APIs to draw circles on the terminal with Assignment 7.
// In particular the user should call either:
// (1) PlotCircle 
//   or
// (2) ClearCircle
void CircleDrawUtil(int XC, int YC, int X, int Y, int Color, char Sym) {
  PlotChar(XC + X, YC + Y, Color, Sym);
  PlotChar(XC - X, YC + Y, Color, Sym);
  PlotChar(XC + X, YC - Y, Color, Sym);
  PlotChar(XC - X, YC - Y, Color, Sym);
  PlotChar(XC + Y, YC + X, Color, Sym);
  PlotChar(XC - Y, YC + X, Color, Sym);
  PlotChar(XC + Y, YC - X, Color, Sym);
  PlotChar(XC - Y, YC - X, Color, Sym);
}

void GeneralizedCircle(int XC, int YC, int R, int Color, char Sym) {
  int X = 0;
  int Y = R;
  int D = 3 - (2*R);

  CircleDrawUtil(XC, YC, X, Y, Color, Sym);
  while (Y >= X) {         
    X++;
    if (D > 0) {
      Y--; 
      D = D + 4 * (X - Y) + 10;
    } else {
      D = D + 4 * X + 6;
    }
    CircleDrawUtil(XC, YC, X, Y, Color, Sym);
  }
}

void PlotCircle(int X, int Y, int R, int Color) {
  GeneralizedCircle(X, Y, R, Color, '+');
}

void ClearCircle(int X, int Y, int R) {
  GeneralizedCircle(X, Y, R, BLACK, ' ');
}


void PlotLine(int X0, int Y0, int X1, int Y1, int Color) {
  GeneralizedPlotLine(X0, Y0, X1, Y1, Color, '*');
}


void ClearLine(int X0, int Y0, int X1, int Y1) {
  GeneralizedPlotLine(X0, Y0, X1, Y1, BLACK, ' ');
}
/* END PLOT UTILITIES */

#endif