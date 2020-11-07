#include "config.h"
#include <math.h>
#include <opencv2/opencv.hpp>
#include <softPwm.h>
#include <stdio.h>
#include <wiringPi.h>

using namespace cv;

#define LOG(msg) printf(msg "\n");

// Motor control
#define PWM_MAX 128
#define PUNCH_THRESH 0.5f
#define PUNCH_T 2

// Image processing
#define IMAGE_DOWNSIZE 4
#define IMAGE_ROI 2 / 3
#define COLOR_MEAN 30
#define COLOR_RANGE 5

// Drive control
#define POS_GAIN 3.5f
#define VELO_MAIN 0.7f

// Log / Debugging
#define DEBUG_CAPTURE 0
#define DEBUG_LOG 1

int pins[4] = {PIN_L_A,
               PIN_L_B,
               PIN_R_A,
               PIN_R_B};

void init()
{
  // Set all pins to high
  for (int i = 0; i < 4; i++)
  {
    pinMode(pins[i], OUTPUT);
    softPwmCreate(pins[i], PWM_MAX, PWM_MAX);
  }
}

// Set velocity of one wheel
void setVeloWheel(int pinBASE, int pinSGN, float value)
{
  // If value is minus, swap pin and use abs(value)
  if (value < 0)
  {
    int tmp = pinBASE;
    pinBASE = pinSGN;
    pinSGN = tmp;
    value = -value;
  }

  // Set base pin to 1, because L9110s motordriver uses pull-up.
  softPwmWrite(pinBASE, PWM_MAX);

  // If value is too small, punch.
  if (value < PUNCH_THRESH)
  {
    softPwmWrite(pinSGN, 0);
    delay(PUNCH_T);
  }

  // Calculate pwm (we inverse the value because the signal is 0, not 1.)
  int pwm = (int)(PWM_MAX * (1 - value));
  if (pwm > PWM_MAX)
  {
    pwm = PWM_MAX;
  }

  // Apply pwm
  softPwmWrite(pinSGN, pwm);
}

// Set velocity of both wheels
void setVelo(float left, float right)
{
  setVeloWheel(PIN_L_A, PIN_L_B, left);
  setVeloWheel(PIN_R_A, PIN_R_B, right);
}

int main(void)
{
  LOG("Check wiringPi setup");
  if (wiringPiSetup() == -1)
    return 1;

  LOG("Initialize software pwm");
  init();

  LOG("Open video");
  VideoCapture cap(0);

  // Variables for image processing
  Mat org, hsv, mask, hsvSplit[3];
  cap.read(org);
  Size sizeOrg = org.size();
  Size sizeSmall = Size(sizeOrg.width / IMAGE_DOWNSIZE, sizeOrg.height / IMAGE_DOWNSIZE);

  // Variables for drive control
  int i, y, x, weightSum, positionSum;
  float position, left, right, posBef;

  // Variables for debugging
  char posBar[21];
  posBar[20] = 0;
  posBef = 0;

  LOG("Start linetracing");
  for (;;)
  {
    // Read image from camera
    cap.read(org);

    // Preprocess
    resize(org, org, sizeSmall);
    rotate(org, org, ROTATE_180);
    cvtColor(org, hsv, COLOR_BGR2HSV);
    split(hsv, hsvSplit);

    // Get lane mask
    bitwise_and((COLOR_MEAN - COLOR_RANGE) < hsvSplit[0], hsvSplit[0] < (COLOR_MEAN + COLOR_RANGE), mask);

#if DEBUG_CAPTURE
    imwrite("test.jpg", mask);
    return 0;
#endif

    // Calculate lane position (weighted sum of lane pixels)
    weightSum = 0;
    positionSum = 0;
    for (y = sizeSmall.height * IMAGE_ROI; y < sizeSmall.height; y++)
    {
      uchar *row = mask.ptr<uchar>(y);
      for (x = 0; x < sizeSmall.width; x++)
      {
        weightSum += row[x];
        positionSum += row[x] * x;
      }
    }
    // Update position only if there are significantly many lane pixels.
    if (weightSum > 50)
    {
      position = positionSum * 1.f / weightSum; // Calculated weighted mean
      position /= sizeSmall.width;              // Normalize
      position -= 0.5f;                         // Remove bias
      position *= POS_GAIN;                     // Apply gain
    }

    // Calculated wheel velocity from position
    left = VELO_MAIN * (1 + position);
    right = VELO_MAIN * (1 - position);

    // Apply wheel velocity
    setVelo(left, right);

#if DEBUG_LOG
    int intPos = (position + 1) * 10;
    if (intPos < 0)
      intPos = 0;
    if (intPos > 19)
      intPos = 19;
    for (i = 0; i < 20; i++)
    {
      if (intPos == i)
        posBar[i] = 'X';
      else if (i == 10)
        posBar[i] = '|';
      else
        posBar[i] = ' ';
    }
    printf("%s : %+2.2f\t%+2.2f\n", posBar, left, right);
#endif
  }
  return 0;
}