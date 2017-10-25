#include <Arduino.h>
#include <CMMC_LED.h>

CMMC_LED led1(1, LOW);
CMMC_LED led2(2, LOW);
CMMC_LED led3(3, LOW);

void setup()
{
  led1.init();
  led2.init();
  led3.init();
}

void loop()
{
  led1.toggle();
  led2.toggle();
  led3.toggle();
}
