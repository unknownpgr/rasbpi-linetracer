#include "config.h"
#include <stdio.h>
#include <wiringPi.h>

int main(void)
{
  printf("Raspberry Pi blink\n");

  if (wiringPiSetup() == -1)
    return 1;

  int pins[4] = {
      PIN_L_A,
      PIN_L_B,
      PIN_R_A,
      PIN_R_B};

  for (int i = 0; i < 4; i++)
    pinMode(pins[i], OUTPUT);

  for (int i = 0;; i++)
  {
    i %= 4;
    printf("Activate : pins[%d]=%d\n", i, pins[i]);
    for (int j = 0; j < 4; j++)
    {
      if (i == j)
      {
        digitalWrite(pins[j], 0);
      }
      else
      {
        digitalWrite(pins[j], 1);
      }
    }
    delay(1000);
  }
  return 0;
}