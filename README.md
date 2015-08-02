TinyLEDStripPWM
===============

MCU to drive a 12V analog LED Strip with PWM controlled over I2C.
It is intendend to be a companion for my branch of [shairport](https://github.com/kwasmich/shairport).

Runs the ATtiny25/45/85 at 16MHz and uses the Timer/Counter in CTC-mode to generate soft-PWM signals to drive the LEDs.
The LED runs at effectively 500Hz. To further reduce flicker the PWM algorithm has been altered for shorter duty cycles
so the LED is driven at 2kHz.

The MCU is addressed via I2C. The initial address is `0x3f`.
The following registers are accessible:

`@ (0x40)` - I2C-Address (MCU reboots after this is set)

`R (0x52)` - Initial value for r

`G (0x47)` - Initial value for g

`B (0x42)` - Initial value for b

`r (0x72)` - Current value for r

`g (0x67)` - Current value for G

`b (0x62)` - Current value for B

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
