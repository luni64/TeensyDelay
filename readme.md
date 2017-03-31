# TeensyDelay
## Problem to be solved 


Some external devices need rather long control pulses to operate correctly. Especially devices with opto-coupled control inputs like power stepper motor drivers can require pulse lengths in the 10µs range. Even a simple DRV8825 driver requires a pulse width of about 2-3µs for a reliable operation. The straight forward approach to generate such pulses would be:
```c++
void pulse()
{
	digitalWriteFast(somePIN,HIGH);
	delayMicroseconds(10);
	digitalWriteFast(somePIN,LOW);
}
```
This is fine as long as you don't need high repetition rates. However, in real life situations the seemingly harmless delay of 10µs can generate quite some load on the processor. Lets assume that we want to use a Leadshine DM422 to drive a stepper motor at a speed of 1000 rpm with a mircostep resolution of 1/32. With the usual motor resolution of 200 full steps/rev we need a pulse rate *r* of

&emsp;&emsp;&emsp;*r* = 200 * 32 * 1000 / 60 = 106'666  steps per second.

Since the driver requires a pulse width of 7.5µs this sums up to a processor load of

&emsp;&emsp;&emsp; *load* = 106'666 steps/s * 7.5 µs/step = 80%.

I.e, in this example the simple task of driving a stepper with 1000rpm would  keep your processor busy for **80%** of the time. 

## Purpose of the Library
This problem can easily be solved  by the usual procedure
- Set the step pin to HIGH
- Calculate timer overflow value to generate the required delay and start timer
- Reset the step pin during the timer interrupt service routine
- Stop the timer

**TeensyDelay** provides an easy to use interface to perform this task without requiring the user to fiddle around with interrupt programming. It does not waste one of the 'valuable' 32bit PIT timers but uses one of the hardly used FTM or TPM timers instead. It provides up to 8 independent delay channels. **TeensyDelay** is compatible to Teensy LC, Teensy 3.0, Teensy 3.1/3.2, Teensy 3.5 and Teensy 3.6. 

## Usage
###  Basic example
The following code demonstrates the use of TeensyDelay.

- **setup**: After initializing the library with the usual call to *begin*  we call *addChannel* to add a new delay channel and attach a callback function to it. 
- **loop**; After switching on the builtin LED we start the delay channel by calling *TeensyDelay::trigger* with a delay time of 25ms. The trigger function will take care of setting up and starting the interrupt timer so that the callback function will be called after the chosen delay time. The callback function of this simple example does nothing more than switching off the LED.  
```c++
#include "TeensyDelay.h"

//Define a function which is called after the delay period has expired
void callback() 
{
    digitalWriteFast(LED_BUILTIN,LOW);
}

void setup()
{
    TeensyDelay::begin();                 // setup timer
    TeensyDelay::addChannel(callback);    // add a delay channel and attach callback function
}

void  loop()
{
    digitalWriteFast(LED_BUILTIN,HIGH);   // switch on LED
    TeensyDelay::trigger(25000);          // trigger the delay channel, callback will be invoked 25ms after this call
    
    delay(1000);                     
}
```
## Performance
The library is optimized for speed (obviously, it wouldn't make much sense to use an interrupt driven pulse generation if the interrupt and trigger functions would eat up the same processor time as the simple delay in the first code block above)

To anlayze the performance of the library I added code to set pin 0 HIGH during the the trigger function and code to set pin 1 HIGH during theinterrupt service routine. Trigger delay was set to 10µs. The shown measurement was done using a Teensy 3.6.


![Alt text](/media/timing.PNG?raw=true "Logic analyzer trace")

 The figure above shows that the time spent in the trigger function was about 0.25µs, the time spent in the interrupt service routine (including the callback) was about 0.38µs. In total we used 0.63µs to generate the 10µs pulse. For the stepper driver example calculated above we would now get a processor load of 
 
 &emsp;&emsp;&emsp; *l* = 106'666 steps/s * 0.63 µs/step = 7%.
 
 instead of the 80% using the simple delay. 
 
 ## Further information 
 For further information please have a look at the code in the provided examples. 

[//]: ----------------------------------------
   [PJRC]: <https://www.pjrc.com/teensy/pinout.html>

