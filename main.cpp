#include "config.h"
#include <math.h>
#include <opencv2/opencv.hpp>
#include <softPwm.h>
#include <stdio.h>
#include <wiringPi.h>

using namespace cv;

#define LOG(msg) printf(msg "\n");

#define PWM_MAX 128
#define PUNCH_THRESH 0.5
#define PUNCH_T 2

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
void setVeloWheel(int pinGND, int pinSGN, float value)
{
  // If value is minus, swap pin and use abs(value)
  if (value < 0)
  {
    int tmp = pinGND;
    pinGND = pinSGN;
    pinSGN = tmp;
    value = -value;
  }

  softPwmWrite(pinGND, PWM_MAX);
  // If value is too small, punch.
  if (value < PUNCH_THRESH)
  {
    softPwmWrite(pinSGN, 0);
    delay(PUNCH_T);
  }
  if (value > 1)
    value = 1;
  int pwm = (int)(PWM_MAX * (1 - value));
  softPwmWrite(pinSGN, pwm);
}

// Set velocity of each wheel
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

  const int center = 30;
  const int delta = 5;
  const float gain = 3.5;
  Mat org, hsv, mask, hsvSplit[3];

  cap.read(org);

  Size sizeOrg = org.size();
  Size sizeSmall = Size(sizeOrg.width / 4, sizeOrg.height / 4);
  int i, y, x, weightSum, positionSum;
  float position, left, right, posBef;
  char posBar[21];
  posBar[20] = 0;
  posBef = 0;
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
    bitwise_and((center - delta) < hsvSplit[0], hsvSplit[0] < (center + delta), mask);
#if 0
    imwrite("test.jpg", mask);
    return 0;
#endif

    // Calculate lane position
    weightSum = 0;
    positionSum = 0;
    for (y = sizeSmall.height * 2 / 3; y < sizeSmall.height; y++)
    {
      uchar *row = mask.ptr<uchar>(y);
      for (x = 0; x < sizeSmall.width; x++)
      {
        weightSum += row[x];
        positionSum += row[x] * x;
      }
    }
    if (weightSum > 50)
    {
      position = positionSum * 1.f / weightSum;
      position /= sizeSmall.width; // Normalize
      position -= 0.5f;            // Remove bias
      position *= gain;            // Gain
      if (position > 0.98f)
        position = 0.98f;
      if (position < -0.98f)
        position = -0.98f;
    }

    // posBef = position * 0.5f + posBef * 0.5f;
    // position = posBef;

    left = 0.7f * (1 + position);
    right = 0.7f * (1 - position);

#if 1
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

    setVelo(left, right);
  }
  return 0;
}