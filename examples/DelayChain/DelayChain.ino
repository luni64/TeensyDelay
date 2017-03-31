//-------------------------------------------------------
// The following sketch shows how to chain two delay channels:
// Channel 0 is triggered periodically in loop. The callback 
// function of channel 0 clears pin 0 and triggers delay channel 1
// callback function of channel 1 clears pin 1.
// A recording of the genreated signals is shown in the attached jpgs. 
//--------------------------------------------------------


#include <TeensyDelay.h>

// callback function for channel 0
void callback_A()
{
  digitalWriteFast(0,LOW);      // clear pin 0
  TeensyDelay::trigger(200,1);  // trigger channel 1, delay: 200µs, 
}

// callback function for channel 0
void callback_B()
{
  digitalWriteFast(1,LOW);      // clear pin 1
}


void setup() 
{ 
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT); 

  TeensyDelay::begin();
  TeensyDelay::addDelayChannel(callback_A,0); //setup channel 0 
  TeensyDelay::addDelayChannel(callback_B,1); //setup channel 1 
} 

void loop() 
{
    digitalWriteFast(0,HIGH);   // set pin 0 and
    digitalWriteFast(1,HIGH);   // pin 1 
    TeensyDelay::trigger(20,0); // trigger channel 0. (channel 1 will be triggered
                                // in the callback function of channel 0. 
  
    delay(1);                   // repeat every millisecond
}



