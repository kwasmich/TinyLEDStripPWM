TinyLEDStripPWM
===============

MCU to drive a 12V analog LED Strip with PWM controlled over I2C.
It is intendend to be a companion for my branch of [shairport](https://github.com/kwasmich/shairport).


Build Requirements
------------------

The I2C communication relies on another project which needs to be checked out into the projects folder via:

```
svn co http://usi-i2c-slave.googlecode.com/svn/trunk/ i2c
```


How to use it
-------------

TinyLEDStripPWM the ATtiny25/45/85 at 16MHz and uses the Timer/Counter in CTC-mode to generate soft-PWM signals to drive
the LEDs. The LED runs at 500Hz. To further reduce flicker the PWM algorithm has been altered for shorter duty cycles so
the LED is effectively driven at 2kHz (for values > 3 and < 252).

The MCU is accessed via I2C. The initial address is `0x3f`.
The following registers are available:

`'@' (0x40)` - I2C-Address (MCU reboots after this is set)

`'R' (0x52)` - Initial value for r

`'G' (0x47)` - Initial value for g

`'B' (0x42)` - Initial value for b

`'r' (0x72)` - Current value for r

`'g' (0x67)` - Current value for g

`'b' (0x62)` - Current value for b

All registers can be read and written to but only `@`, `R`, `G` and `B` are persistent via EEPROM.



Wiring
------

```
 VCC   SCK    B    SDA
  |     |     |     |
-----------------------
|                     |
|                     |
|   ATtiny25/45/85    |
|                     |
|                     |
-----------------------
  |     |     |     |
RESET   G     R    GND
```
