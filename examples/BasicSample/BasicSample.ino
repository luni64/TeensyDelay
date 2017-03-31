#include <TeensyDelay.h>

//This callback function will be called after the delay period
void callback()
{
  digitalWriteFast(LED_BUILTIN,LOW);
}

void setup() 
{
  pinMode(LED_BUILTIN,OUTPUT);

  TeensyDelay::begin();
  TeensyDelay::addDelayChannel(callback); // setup a delay channel and attach the callback function to it
}

void loop() 
{
    digitalWriteFast(LED_BUILTIN,HIGH);  // switch on LED
    TeensyDelay::trigger(10000);         // the callback function will switch it off 10ms later

    delay(500);                          // repeat every 500ms
}
