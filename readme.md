# TeensyDelay
## Problem to be Solved 


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
This problem can easily be solved  by using a timer interrupt to generate the pulse:
- Set the pulse pin to HIGH
- Calculate timer overflow value to generate the required delay 
- Start timer with calculated overflow value
- Reset the pulse pin in the timer interrupt service routine
- Stop the timer

**TeensyDelay** provides an easy to use interface to perform this task without requiring the user to fiddle around with interrupt programming. Additionally it does not waste any of the 'valuable' 32bit PIT timers but uses one of the hardly used FTM or TPM timers instead (selectable). Depending on the chosen timer it provides up to 8 independent delay channels. Depending on the selected timer module (see chapter Configuration below) and the processor frequency, the maximum delay time is limited to about 30ms. 
**TeensyDelay** is compatible to T Teensy 3.0, Teensy 3.1/3.2, Teensy 3.5 and Teensy 3.6. Compatibility to Teensy LC will be added later.

(The library [TeensyStep](https://github.com/luni64/TeensyStep) - an efficient stepper motor library- uses **TeensyDelay** for resetting the STEP signals of the driven motors and for generating a periodic interrupt for recalculation of acceleration parameters.)

## Usage
###  Basic Example
The following code demonstrates the use of TeensyDelay.

- **setup**: After initializing the library with the usual call to *begin*  we call *addDelayChannel* to add a new delay channel and attach a callback function to it. 
- **loop**; After switching on the builtin LED we start the delay channel by calling *TeensyDelay::trigger* with a delay time of 25ms. The trigger function will take care of setting up and starting the interrupt timer so that the callback function will be called after the chosen delay time. The callback function in this simple example does nothing more than switching off the LED.  
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
    TeensyDelay::addDelayChannel(callback);    // add a delay channel and attach callback function
}

void  loop()
{
    digitalWriteFast(LED_BUILTIN,HIGH);   // switch on LED
    TeensyDelay::trigger(25000);          // trigger the delay channel, callback will be invoked 25ms after this call
    
    delay(1000);                     
}
```
###  Using more than one Delay Channel
Depending on the chosen timer **TeensyDelay** handles up to eight independent delay channels. Per default **TeensyDelay** uses the first channel of the selected timer. If you need more than one channel the *addDelayChannel* function and the *trigger* function accept an optional parameter to choose one of the channels. 

```c++
	constexpr int ch_A = 6;
	...
	TeensyDelay::addDelayChannel(firstCallback, ch_A);  // Setup channel #6
	TeensyDelay::addDelayChannel(anotherCallback, 2);   // Setup channel #2
	...
	TeensyDelay::trigger(25, 2);                   // Trigger channel #2, 25µs
	TeensyDelay::trigger(20000, ch_A);             //Trigger channel #6, 20ms
	....
```
For performance reasons of the trigger function I recommend to use compile time constants (integer literals, constexpr int or #define) for the channel number (see code above for an example). 

### A Poor Man's Periodic Timer
TeensyDelay was designed for calling a callback function some time after the channel was triggered. However, nothing speaks against triggering a channel within its own callback function. In fact, that is a quick and easy solution if you need a simple periodic timer and don't want to spend one of the IntervalTimers for it. 
```c++	
	void periodicCallback()
	{
	   trigger(10000);  // retrigger the channel (10ms)
	   doSomething();
	}
	
	void setup()
	{
	   TeensyDelay::addDelayChannel(periodicCallback);
	   TeensyDelay::trigger(300);  // start the periodic timer in 300µs
	   ...
	}
	...
```
### Long Delays
Internally TeensyDelay uses one of the FTM or TPM timer modules of the processor. The code in config.h tries to calculate the timer prescaler such that one timer tick corresponds to about 1µs. Since the timer registers of the FTM or TPM modules are only 16bit wide the  delay time is limited to about 65ms. In case you need longer delays the following snippet shows how to achieve five seconds by choosing a base delay of 25ms and retriggering the timer from within the callback function:
```c++	
void callback(){
    static int cnt = 0;
    if (cnt < 200){                    // ignore 200 trigger events (5s = 200 x 25ms)
        cnt++;
        TeensyDelay::trigger(25000);   // retrigger the timer (25ms)
    }
    else{                              // do whatever needs to be done after 5s
        cnt = 0;                                      
        digitalWriteFast(LED_BUILTIN, LOW);
    }
}
void setup(){
    pinMode(LED_BUILTIN, OUTPUT);

    TeensyDelay::begin();                
    TeensyDelay::addDelayChannel(callback);

    digitalWriteFast(LED_BUILTIN, HIGH);
    TeensyDelay::trigger(1);           // this will switch off the LED after 5s;         
}
void  loop(){    
}
```

## Performance
The library is optimized for speed. Obviously, it wouldn't make much sense to use interrupt driven pulse generation if the interrupt and trigger functions would eat up the same processor time as the simple delay in the first code block above.

To anlayze the performance of the library I added code to set pin 0 HIGH during the the trigger function and code to set pin 1 HIGH during the interrupt service routine. Trigger delay was set to 10µs. The shown measurement was done using a Teensy 3.6 @240MHz. 


![Alt text](/media/timing.PNG?raw=true "Logic analyzer trace")

The figure above shows that the time spent in the trigger function was about 0.25µs, the time spent in the interrupt service routine (including the callback) was about 0.38µs. In total we used 0.63µs to generate the 10µs pulse. For the stepper driver example calculated above we would now get a processor load of 
 
 &emsp;&emsp;&emsp; *load* = 106'666 steps/s * 0.63 µs/step = 7%.
 
 instead of the 80% using the simple delay. 
 

## Configuration

Depending on the board type **TeensyDelay** can work with any of the timers shown in the table below.  All timers marked with X are available for the given board. The default timer used by **TeensyDelay**  is marked with D. 

|TIMER |Channels|T3.0|T3.1|T3.2|T3.5|T3.6|                
|------|:------:|:--:|:--:|:--:|:--:|:--:|
|FTM 0 |8       | D  | D  | D  | D  | D  |
|FTM 1 |2       | X  | X  | X  | X  | X  |
|FTM 2 |2       |    | X  | X  | X  | X  |
|FTM 3 |8       |    |    |    | X  | X  |
|TPM 0 |6       |    |    |    |    |    |
|TPM 1 |2       |    |    |    |    | X  |
|TPM 2 |2       |    |    |    |    | X  |

If you want to change the timer used by **TeensyDelay**  please edit line 22 in the file config.h according to the description given in the file. 

**Why would you want to change the default timer?** 
Some libraries require one or more of the FTM/TPM timers for their own purpose. To use those libraries together with TeensyDelay you can try to change the timer used by TeensyDelay. For example the PWM/analogWrite() functionality of Teensyduino might clash with TeensyDuino. You can find information about which analog pin uses which timer module here  https://www.pjrc.com/teensy/td_pulse.html (scroll down to the table in chapter 'PWM Frequency'). 

 ## Further information 
 For further information please have a look at the code in the provided examples and the source files. 

[//]: ----------------------------------------
   [PJRC]: <https://www.pjrc.com/teensy/pinout.html>
