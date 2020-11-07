#include "config.h"
#include <stdio.h>
#include <wiringPi.h>

int main(void)
{
  printf("Release all pins\n");

  if (wiringPiSetup() == -1)
    return 1;

  int pins[4] = {
      PIN_L_A,
      PIN_L_B,
      PIN_R_A,
      PIN_R_B};

  for (int i = 0; i < 4; i++)
    pinMode(pins[i], INPUT);
  return 0;
}