//
//  main.c
//  WaterAlert
//
//  Created by Michael Kwasnicki on 11.09.14.
//  Copyright (c) 2014 Kwasi-ich. All rights reserved.
//


#ifndef F_CPU
#define F_CPU 16000000UL // ATTiny85 has 16MHz
#endif


#include "USI_TWI_Slave.h"
#include "i2c/usiTwiSlave.h"


#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <iso646.h>
#include <stdbool.h>



/*
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
 */

// find the devices address:
//   i2cdetect -y 1
// set green color to 50%
//   i2cset -y 1 0x3f 0x67 127


#define SDA     PB0
#define SCL     PB2
#define G_PIN   PB3
#define R_PIN   PB4
#define B_PIN   PB1

#define _BV2( A, B ) ( _BV( A ) bitor _BV( B ) )
#define _BV3( A, B, C ) ( _BV2( A, B ) bitor _BV( C ) )
#define _BV4( A, B, C, D ) ( _BV2( A, B ) bitor _BV2( C, D ) )
#define _BV5( A, B, C, D, E ) ( _BV2( A, B ) bitor _BV3( C, D, E ) )
#define BIT_SET( PORT, BIT_FIELD ) PORT |=  BIT_FIELD
#define BIT_CLR( PORT, BIT_FIELD ) PORT &= ~BIT_FIELD
#define BIT_TGL( PORT, BIT_FIELD ) PORT ^=  BIT_FIELD


volatile uint8_t cnt = 0;
volatile uint8_t r = 0;
volatile uint8_t g = 0;
volatile uint8_t b = 0;



static void pwm() {
//    if ( cnt == 0 ) {
//        if ( r > cnt ) BIT_SET( PORTB, _BV( R_PIN ) );
//        if ( g > cnt ) BIT_SET( PORTB, _BV( G_PIN ) );
//        if ( b > cnt ) BIT_SET( PORTB, _BV( B_PIN ) );
//    }
//    
//    if ( cnt >= r ) {
//        BIT_CLR( PORTB, _BV( R_PIN ) );
//    }
//    
//    if ( cnt >= g ) {
//        BIT_CLR( PORTB, _BV( G_PIN ) );
//    }
//    
//    if ( cnt >= b ) {
//        BIT_CLR( PORTB, _BV( B_PIN ) );
//    }
//    
//    cnt++;
    
    if ( cnt < 4 ) {
        if ( r > cnt ) BIT_SET( PORTB, _BV( R_PIN ) );
        if ( g > cnt ) BIT_SET( PORTB, _BV( G_PIN ) );
        if ( b > cnt ) BIT_SET( PORTB, _BV( B_PIN ) );
    }
    
    if ( cnt >= r ) {
        BIT_CLR( PORTB, _BV( R_PIN ) );
    }
    
    if ( cnt >= g ) {
        BIT_CLR( PORTB, _BV( G_PIN ) );
    }
    
    if ( cnt >= b ) {
        BIT_CLR( PORTB, _BV( B_PIN ) );
    }
    
    switch ( cnt ) {
        case 252:   cnt = 2;    break;
        case 253:   cnt = 3;    break;
        case 254:   cnt = 1;    break;
        case 251:   cnt = 0;    break;
        default:    cnt += 4;   break;
    }
}



ISR( TIM0_COMPA_vect ) {
    pwm();
}



static void wdt_off( void ) {
    wdt_reset();                            // reset watchdog
    MCUSR = 0x00;                           // clear reset status
    BIT_SET( WDTCR, _BV2( WDCE, WDE ) );    // needs to be set in order to make changes to the prescaler
    WDTCR = 0x00;                           // turn watchdog off
}



static void wdt_on( void ) {
    BIT_SET( WDTCR, _BV2( WDCE, WDE ) );    // needs to be set in order to make changes to the prescaler
    WDTCR = _BV( WDE );                     // set timer to 16ms and enable reset timer
}



static uint8_t eeprom_read(uint8_t ucAddress) {
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE));
    /* Set up address register */
    EEAR = ucAddress;
    /* Start eeprom read by writing EERE */
    EECR |= (1<<EERE);
    /* Return data from data register */
    return EEDR;
}



static void eeprom_write( uint8_t ucAddress, uint8_t ucData ) {
    /* Wait for completion of previous write */
    while(EECR & (1<<EEPE));
    /* Set Programming mode */
    EECR = (0<<EEPM1)|(0<<EEPM0);
    /* Set up address and data registers */
    EEAR = ucAddress;
    EEDR = ucData;
    /* Write logical one to EEMPE */
    EECR |= (1<<EEMPE);
    /* Start eeprom write by setting EEPE */
    EECR |= (1<<EEPE);
}



static void eeprom_init() {
    uint8_t i2cAddr = eeprom_read( 0x00 );
    
    // if the I2C-Address is invalid then write sane values into the EEPROM
    if ( i2cAddr < 0x03 or i2cAddr > 0x77 ) {
        eeprom_write( 0x00, 0x3f );
        eeprom_write( 0x01, 0x00 );
        eeprom_write( 0x02, 0x00 );
        eeprom_write( 0x03, 0x00 );
    }
}



static void reset() {
    wdt_on();   // reset via watchdog
    
    while (true) {}
}



// A callback triggered when the i2c master attempts to read from a register.
static uint8_t i2cReadFromRegister(uint8_t reg)
{
    switch( reg ) {
        case '@': return eeprom_read( 0x00 );   // this is pretty much useless but for completeness
        case 'R': return eeprom_read( 0x01 );
        case 'G': return eeprom_read( 0x02 );
        case 'B': return eeprom_read( 0x03 );
        case 'r': return r;
        case 'g': return g;
        case 'b': return b;
        default: return 0;
    }
}



// A callback triggered when the i2c master attempts to write to a register.
static void i2cWriteToRegister( uint8_t reg, uint8_t value ) {
    switch( reg ) {
        case '@': eeprom_write( 0x00, value ); reset(); break;
        case 'R': eeprom_write( 0x01, value ); break;
        case 'G': eeprom_write( 0x02, value ); break;
        case 'B': eeprom_write( 0x03, value ); break;
        case 'r': r = value;  break;
        case 'g': g = value;  break;
        case 'b': b = value;  break;
    }
}



int main(void) {
    wdt_off();                          // recover from watchdog reset
    
    eeprom_init();                      // sets eeprom to sane values
    
    DDRB = _BV3( R_PIN, G_PIN, B_PIN ); // Set up output pins
    
    TCCR0A |= _BV( WGM01 );             // enable CTC
    OCR0A = 128;                        // twice as often as with overflow
    TCCR0B |= _BV( CS00 );              // prescale timer to the clock rate
    TIMSK |= _BV( OCIE0A );             // enable compare match interrupt
    
    uint8_t i2cAddr = eeprom_read( 0x00 );
    usiTwiSlaveInit( i2cAddr, i2cReadFromRegister, i2cWriteToRegister );
    
    r = eeprom_read( 0x01 );
    g = eeprom_read( 0x02 );
    b = eeprom_read( 0x03 );
    
    sei();
    
    while( true ) {
        // let ISR handle the LED forever
    }
}
