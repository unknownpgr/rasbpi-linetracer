#include <stdio.h>
#include <wiringPi.h>

#define PIN_L_A 7
#define PIN_L_B 0
#define PIN_R_A 2
#define PIN_R_B 3

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
    pinMode(pins[i], INPUT);
  return 0;
}