#include "config.h"
#include <math.h>
#include <opencv2/opencv.hpp>
#include <softPwm.h>
#include <stdint.h>
#include <stdio.h>
#include <wiringPi.h>

using namespace cv;

#define LOG(msg) printf(msg "\n");

// Motor control
#define PWM_MAX      128
#define PUNCH_THRESH 0.5f
#define PUNCH_T      0

// Image processing
#define IMAGE_DOWNSIZE 4
#define IMAGE_ROI      0.8f
#define COLOR_MEAN     30
#define COLOR_RANGE    5

// Drive control
#define POS_GAIN_P 2.5f
#define POS_GAIN_I 0.00f
#define VELO_MAIN  0.115f

// Log / Debugging
#define DEBUG_CAPTURE 0
#define DEBUG_LOG     1

int pins[4] = {PIN_L_A, PIN_L_B, PIN_R_A, PIN_R_B};

void init() {
  // Set all pins to high
  for (int i = 0; i < 4; i++) {
    pinMode(pins[i], OUTPUT);
    softPwmCreate(pins[i], PWM_MAX, PWM_MAX);
  }
}

// Set velocity of one wheel
void setVeloWheel(int pinBASE, int pinSGN, float value) {
  // If value is minus, swap pin and use abs(value)
  if (value < 0) {
    int tmp = pinBASE;
    pinBASE = pinSGN;
    pinSGN = tmp;
    value = -value;
  }
  if (value > 1)
    value = 1;

  int time_full = 100;
  int time_punch = time_full * value;
  int time_sleep = time_full - time_punch;

  // Set base pin to 1, because L9110s motordriver uses pull-up.
  softPwmWrite(pinBASE, PWM_MAX);
  softPwmWrite(pinSGN, 0);
  delay(time_punch);
  softPwmWrite(pinSGN, PWM_MAX);
  // delay(time_sleep);
}

// Set velocity of both wheels
void setVelo(float left, float right) {
  setVeloWheel(PIN_L_A, PIN_L_B, left);
  setVeloWheel(PIN_R_A, PIN_R_B, right);
  delay(70);
}

int main(void) {
  LOG("Check wiringPi setup");
  if (wiringPiSetup() == -1)
    return 1;

  LOG("Initialize software pwm");
  init();

  LOG("Open video");
  VideoCapture cap(0);

  // Variables for image processing
  Mat org, small, crop, gray;
  cap.read(org);
  Size sizeOrg = org.size();
  Size sizeSmall =
      Size(sizeOrg.width / IMAGE_DOWNSIZE, sizeOrg.height / IMAGE_DOWNSIZE);
  int _roi = sizeSmall.height * IMAGE_ROI;
  int _h = sizeSmall.height - _roi;
  Rect ROI_RECT(0, _roi, sizeSmall.width, _h);

  printf("ROI:%d\n", _roi);

  // Variables for drive control
  int32_t weightSum, positionSum;
  int i, y, x;
  float position, left, right, posBef, err_i = 0, control;

  // Variables for debugging
  char posBar[21];
  posBar[20] = 0;
  posBef = 0;

  LOG("Start linetracing");
  for (;;) {
    // Preprocess
    resize(org, small,
           sizeSmall); // Reduce the image size for computation time.
    rotate(small, small, ROTATE_180); // Rotate the image because my camera is
    crop = small(ROI_RECT);           // attached upside-down.
    cvtColor(crop, gray, COLOR_BGR2GRAY);
    // adaptiveThreshold(gray, gray, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,
    //                   11, 25);
    threshold(gray, gray, 128, 255, THRESH_OTSU);

    // Calculate lane position (weighted sum of lane pixels)
    weightSum = 0;
    positionSum = 0;
    for (y = 0; y < _h; y++) {
      uchar *row = gray.ptr<uchar>(y);
      for (x = 0; x < sizeSmall.width; x++) {
        int value = 255 - row[x];
        row[x] = value;
        weightSum += value;
        positionSum += value * x;
      }
    }

    // for (y = 0; y < _h; y++) {
    //   uchar *row = gray.ptr<uchar>(y);
    //   row[positionSum / weightSum] = 0;
    // }

    imwrite("test.jpg", gray);

    // Update position only if there are significantly many lane pixels.
    if (weightSum > 300) {
      position = positionSum * 1.f / weightSum; // Calculated weighted mean
      position /= sizeSmall.width;              // Normalize
      position -= 0.5f;                         // Remove bias
      position *= POS_GAIN_P;                   // Apply gain
    }

    err_i += position * POS_GAIN_I;
    control = position;

    // Calculated wheel velocity from position
    left = VELO_MAIN * (1 + control);
    right = VELO_MAIN * (1 - control);

    // Read image from camera
    cap.read(org);

    // Apply wheel velocity
    setVelo(left, right);

#if DEBUG_LOG
    int intPos = (position + 1) * 10;
    if (intPos < 0)
      intPos = 0;
    if (intPos > 19)
      intPos = 19;
    for (i = 0; i < 20; i++) {
      if (intPos == i)
        posBar[i] = 'X';
      else if (i == 10)
        posBar[i] = '|';
      else
        posBar[i] = ' ';
    }
    printf("%s : %+2.2f\t%+2.2f\t%+2.2f\n", posBar, left, right, control);
#endif
  }

  return 0;
}