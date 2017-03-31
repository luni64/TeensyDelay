# Work in progress
NOT FUNCTIONAL CURRENTLY



# TeensyDelay
## Purpose of the library
Some external devices like stepper motor drivers need rather long control pulses to operate correctly. Especially devices with opto coupled control inputs can require pulse lenghts in the 10µs range. A straight forward approach to this problem would be
```c++
...
digitalWriteFast(somePIN,HIGH)
delayMicroseconds(10)
digitalWriteFast(somePIN,LOW)
...
```
This is fine of course as long as you don't need high repetition rates. Lets assume that we want to operate a stepper motor at 1000rpm with a mircostep resolution of 1/32. If we assume 200 full steps per revolution this requires a pulse rate *r* of *r* = 200 * 32 * 800 / 60 = 85'333  steps per second. 
 
	
What about place a determined space in the start of paragraph using the math environment as like:


<dl>
  <dt>Definition list</dt>
  <dd>Is something people use sometimes.</dd>

  <dt>Markdown in HTML</dt>
  <dd>Does *not* work **very** well. Use HTML <em>tags</em>.</dd>
</dl>



TeensyDelay is an easy to use library providing up to eight indepenedent asynchronous delay channels. 

For all [PJRC] Teensy AMR boards. I.e., Teensy LC, Teensy 3.0, Teensy 3.1/3.2, Teensy 3.5 and Teensy 3.6. Per default a delay from about 1µs up to about 50ms can be choosen independently for each channel. 

The following code demonstrates the use of TeensyDelay. In loop() the LED is switched on every 500ms, TeensyDelay switches it off asynchronically after 25ms. 
```c++
#include <Arduino.h>
#include "TeensyDelay.h"

//Define a function which is called after the delay period has expired
void callback() 
{
    digitalWriteFast(LED_BUILTIN,LOW);
}

void setup()
{
    TeensyDelay::begin();                   // setup timer
    TeensyDelay::AddChannel(0,callback);    // add a delay channel and attach callback function
}

void  loop()
{
    digitalWriteFast(LED_BUILTIN,HIGH);     // switch on LED
        TeensyDelay::Trigger(0, 25000);     // trigger the delay channel, callback will be invoked 25000µs after this call
    
    delay(250);                     
}
```



[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

   [PJRC]: <https://www.pjrc.com/teensy/pinout.html>
