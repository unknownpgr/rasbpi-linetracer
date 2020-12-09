#include "config.h"
#include <math.h>
#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <softPwm.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>

using namespace cv;

#define LOG(msg) printf(msg "\n");
#define ABS(x)   (((x) > 0) ? (x) : (-(x)))

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
#define POS_GAIN_P  2.0f
#define POS_GAIN_I  0.00f
#define VELO_MAIN   0.135f
#define CURV_DECCEL 0.25f

// Log / Debugging
#define DEBUG_CAPTURE 0
#define DEBUG_LOG     0

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

  // Set base pin to 1, because L9110s motordriver uses pull-up.
  softPwmWrite(pinBASE, PWM_MAX);

  // Calculate pwm (we inverse the value because the signal is 0, not 1.)
  int pwm = (int)(PWM_MAX * (1 - value));
  if (pwm > PWM_MAX) {
    pwm = PWM_MAX;
  }

  // Apply pwm
  softPwmWrite(pinSGN, pwm);
}

// Set velocity of both wheels
void setVelo(float left, float right) {
  setVeloWheel(PIN_L_A, PIN_L_B, left);
  setVeloWheel(PIN_R_A, PIN_R_B, right);
  usleep(10 * 1000);
}

float position;

void *thread_imread(void *args) {

  VideoCapture cap;
  cap.open(0);

  Mat org, small, crop, gray;
  cap.read(org);
  Size sizeOrg = org.size();
  Size sizeSmall =
      Size(sizeOrg.width / IMAGE_DOWNSIZE, sizeOrg.height / IMAGE_DOWNSIZE);
  int _roi = sizeSmall.height * IMAGE_ROI;
  int _h = sizeSmall.height - _roi;
  int x, y;
  int _roi_x = 0;
  Rect ROI_RECT(_roi_x, _roi, sizeSmall.width - _roi_x * 2, _h);

  printf("ROI:%d\n", _roi);

  LOG("Open video");

  // Variables for drive control
  int32_t weightSum, positionSum;
  float tempPos;

  while (1) {
    cap.read(org);
    // Preprocess
    resize(org, small,
           sizeSmall); // Reduce the image size for computation time.
    rotate(small, small, ROTATE_180); // Rotate the image because my camera is
    crop = small(ROI_RECT);           // attached upside-down.
    cvtColor(crop, gray, COLOR_BGR2GRAY);
    // adaptiveThreshold(gray, gray, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,
    //                   11, 25);
    threshold(gray, gray, 128, 255, THRESH_OTSU);
    imwrite("test.jpg", gray);
    // exit(0);

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

    // Update position only if there are significantly many lane pixels.
    if (weightSum > 100) {
      tempPos = positionSum * 1.f / weightSum; // Calculated weighted mean
      tempPos /= sizeSmall.width;              // Normalize
      tempPos -= 0.5f;                         // Remove bias
      tempPos *= POS_GAIN_P;                   // Apply gain
      position = tempPos;
    }
  }
}

int main(void) {
  LOG("Check wiringPi setup");
  if (wiringPiSetup() == -1)
    return 1;

  LOG("Initialize software pwm");
  init();

  int i, y, x;
  float left, right, posBef, err_i = 0, control;

  // Variables for debugging
  char posBar[21];
  posBar[20] = 0;
  posBef = 0;

  pthread_t tid;
  pthread_create(&tid, NULL, thread_imread, (void *)&tid);

  LOG("Start linetracing");
  for (;;) {
    control = position;

    // Calculated wheel velocity from position
    static float veloMain;
    veloMain = VELO_MAIN / (1 + ABS(control) * CURV_DECCEL);
    left = veloMain * (1 + control);
    right = veloMain * (1 - control);

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