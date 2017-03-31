# TeensyDelay
## Problem to be solved 
Some external devices need rather long control pulses to operate correctly. Especially devices with opto-coupled control inputs like power stepper motor drivers can require pulse lenghts in the 10µs range. Even a simple DRV8825 drvier requires a pulse width of about 2-3µs for a reliable operation. The straight forward approach to generate such pulses would be:
```c++
...
digitalWriteFast(somePIN,HIGH)
delayMicroseconds(10)
digitalWriteFast(somePIN,LOW)
...
```
This is fine of course as long as you don't need high repetition rates. However, in real life situations the seemingly harmless delay of 10µs can generate quite some load on the processor. Lets assume that we want to use a Leadshine DM422 to drive a stepper motor at a speed of 1000 rpm with a mircostep resolution of 1/32. With the usual motor resulution of 200 full steps/rev we need a pulse rate *r* of

&emsp;&emsp;&emsp;*r* = 200 * 32 * 1000 / 60 = 106'666  steps per second.

Since the driver requires a pulse widht of 7.5µs this sums up to a processor load *l* of

&emsp;&emsp;&emsp; *l* = 106'666 steps/s * 7.5 µs/step = 80%.

I.e, in this example the simple task of driving a stepper with 1000rpm would  keep your processor busy for **80%** of the time. 

## Purpose of the Library
This problem can easily be solved  by the usual procedure
- Set the pin to HIGH
- Start one of the timers (setting correct overflow value to generate the pulse)
- Reset the pin in the interrupt service routine of the timer
- Stop the timer

**TeensyDelay** provides an easy to use interface to perform this task without requiring the user to fiddle around with interrupt programming. It does not waste one of the 'valuable' 32bit PIT timers but uses one of the hardly used FTM or TPM timers instead. It provides up to 8 independent delay channels. **TeensyDelay** is compatible to Teensy LC, Teensy 3.0, Teensy 3.1/3.2, Teensy 3.5 and Teensy 3.6. 

## Usage
The following code demonstrates the use of TeensyDelay. In loop() we switch on the builtin LED, start on
TeensyDelay switches it off asynchronically after 25ms. 
```c++
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
