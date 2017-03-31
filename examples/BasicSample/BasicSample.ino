#include <TeensyDelay.h>

//This function will be called after the delay period.
void callback()
{
  digitalWriteFast(LED_BUILTIN,LOW);
}

void setup() 
{
  pinMode(LED_BUILTIN,OUTPUT);

  TeensyDelay::begin();
  TeensyDelay::addDelayChannel(callback);
}

void loop() 
{
    digitalWriteFast(LED_BUILTIN,HIGH);  // switch on LED
    TeensyDelay::trigger(20);            // the callback function will switch it of 20µs later

    delay(1);                            // repeat every millsecond
}
