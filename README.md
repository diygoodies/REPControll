# REPControll
> Universal HAMrepeater controller.

Based on STM32 bluepill adrduino board. It basically connects two base VHF or UHF radios. 
![](https://github.com/diygoodies/REPControll/blob/master/Schematics/frontview.jpg)

## Usage example

Controller makes a 3 second TX tail with short beep in the end after each transmition, to understand that repeater is available, and message forwarded. 
Every 15 minutes Controller sends on air CALLSIGN of repeater or QTH locator by morse code.
Schematics [https://github.com/diygoodies/REPControll/blob/master/Schematics/REPControll.pdf]
  
  1. Blink on PC13 per 2s 
  2. Second counter by attachSecondsInterrupt
  3. change to your timezone in the sketch;
  4. get Unix epoch time from https://www.epochconverter.com/ ;
  5. last step input the 10 digit number( example: 1503945555) to Serialport ;
  6. the clock will be reset to you wanted.

## Development setup
Code based on BluePill-RTClock-test example of
[https://github.com/rogerclarkmelbourne/Arduino_STM32]
package
