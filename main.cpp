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
  softPwmWrite(pinSGN, (int)(PWM_MAX * (1 - value)));
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

  Mat img;
  cap.read(img);
  imwrite("test.jpg", img);

  return 0;
}