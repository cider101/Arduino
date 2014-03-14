/*
 * SoftSerial.cpp
 *
 *  Created on: 14.03.2014
 *  Author: cider101
 */

#include "SoftSerial.h"

// When set, _DEBUG co-opts pins 11 and 13 for debugging with an
// oscilloscope or logic analyzer.  Beware: it also slightly modifies
// the bit times, so don't rely on it too much at high baud rates
#define _DEBUG 0
#define _DEBUG_PIN1 11
#define _DEBUG_PIN2 13
//
// Includes
//
#include <Arduino.h>
#include "SoftwareSerial.h"
#include "SoftSerialTiming.h"

#define MAX_SOFT_SERIALS 8
SoftSerial *instances[MAX_SOFT_SERIALS];

void replace_instance(SoftSerial *find, SoftSerial *replace) {
	  for(size_t i = 0; i < MAX_SOFT_SERIALS; ++i) {
		  if(instances[i] == find) {
			  instances[i] = replace;
		  }
	  }
}



SoftSerial::SoftSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic /* = false */) :
  _rx_delay_centering(0),
  _rx_delay_intrabit(0),
  _rx_delay_stopbit(0),
  _tx_delay(0),
  _inverse_logic(inverse_logic)
{
	configure(receivePin, transmitPin, inverse_logic);
	replace_instance(NULL, this);

}


SoftSerial::~SoftSerial()
{
  end();
  replace_instance(this, NULL);
}


uint8_t SoftSerial::rx_pin_read()
{
  return *_receivePortRegister & _receiveBitMask;
}

void SoftSerial::tx_pin_write(uint8_t state) {
	if(state == LOW)
	    *_transmitPortRegister &= ~_transmitBitMask;
	else
		*_transmitPortRegister |= _transmitBitMask;
}


size_t SoftSerial::write(uint8_t b)
{
  if (_tx_delay == 0) { //is this check really necessary ?
    setWriteError();
    return 0;
  }

  uint8_t logical_high = _inverse_logic ? LOW : HIGH;
  uint8_t logical_low = _inverse_logic ? HIGH : LOW;

  uint8_t oldSREG = SREG;
  cli();  // turn off interrupts for a clean txmit


  // Write the start bit
  tx_pin_write(logical_low);
  tunedDelay(_tx_delay + XMIT_START_ADJUSTMENT);

  for (byte mask = 0x01; mask; mask <<= 1) {
     (b & mask) ? tx_pin_write(logical_high) : tx_pin_write(logical_low);
     tunedDelay(_tx_delay);
  }

  tx_pin_write(logical_high); // restore pin to natural state


  SREG = oldSREG; // turn interrupts back on
  tunedDelay(_tx_delay);

  return 1;
}


//
// The receive routine called by the interrupt handler
//
void SoftSerial::recv()
{

#if GCC_VERSION < 40302
// Work-around for avr-gcc 4.3.0 OSX version bug
// Preserve the registers that the compiler misses
// (courtesy of Arduino forum user *etracer*)
  asm volatile(
    "push r18 \n\t"
    "push r19 \n\t"
    "push r20 \n\t"
    "push r21 \n\t"
    "push r22 \n\t"
    "push r23 \n\t"
    "push r26 \n\t"
    "push r27 \n\t"
    ::);
#endif

  uint8_t d = 0;

     // Wait approximately 1/2 of a bit width to "center" the sample
    tunedDelay(_rx_delay_centering);
    DebugPulse(_DEBUG_PIN2, 1);

    // Read each of the 8 bits
    for (uint8_t i=0x1; i; i <<= 1)
    {
      tunedDelay(_rx_delay_intrabit);
      DebugPulse(_DEBUG_PIN2, 1);
      uint8_t noti = ~i;
      if (rx_pin_read())
        d |= i;
      else // else clause added to ensure function timing is ~balanced
        d &= noti;
    }

    // skip the stop bit
    tunedDelay(_rx_delay_stopbit);
    DebugPulse(_DEBUG_PIN2, 1);

    if (_inverse_logic)
      d = ~d;

    // if buffer full, set the overflow flag and return
    if(!put(d))
    {
#if _DEBUG // for scope: pulse pin as overflow indictator
      DebugPulse(_DEBUG_PIN1, 1);
#endif
     }


#if GCC_VERSION < 40302
// Work-around for avr-gcc 4.3.0 OSX version bug
// Restore the registers that the compiler misses
  asm volatile(
    "pop r27 \n\t"
    "pop r26 \n\t"
    "pop r23 \n\t"
    "pop r22 \n\t"
    "pop r21 \n\t"
    "pop r20 \n\t"
    "pop r19 \n\t"
    "pop r18 \n\t"
    ::);
#endif
}

//
// Private methods
//

/* static */
inline void SoftSerial::tunedDelay(uint16_t delay) {
  uint8_t tmp=0;

  asm volatile("sbiw    %0, 0x01 \n\t"
    "ldi %1, 0xFF \n\t"
    "cpi %A0, 0xFF \n\t"
    "cpc %B0, %1 \n\t"
    "brne .-10 \n\t"
    : "+r" (delay), "+a" (tmp)
    : "0" (delay)
    );
}

void SoftSerial::configure(uint8_t rx, uint8_t tx, bool inverse_logic) {
	  _inverse_logic = inverse_logic;
	  setTX(rx);
	  setRX(tx);
}

void SoftSerial::setTX(uint8_t tx)
{
  pinMode(tx, OUTPUT);
  digitalWrite(tx, HIGH);
  _transmitBitMask = digitalPinToBitMask(tx);
  uint8_t port = digitalPinToPort(tx);
  _transmitPortRegister = portOutputRegister(port);
}

void SoftSerial::setRX(uint8_t rx)
{
  pinMode(rx, INPUT);
  if (!_inverse_logic)
    digitalWrite(rx, HIGH);  // pullup for normal logic!
  _receivePin = rx;
  _receiveBitMask = digitalPinToBitMask(rx);
  uint8_t port = digitalPinToPort(rx);
  _receivePortRegister = portInputRegister(port);
}

//
// Public methods
//

void SoftSerial::begin(unsigned long speed)
{
  _rx_delay_centering = _rx_delay_intrabit = _rx_delay_stopbit = _tx_delay = 0;

  for (unsigned i=0; i<sizeof(table)/sizeof(table[0]); ++i)
  {
    unsigned long baud = pgm_read_dword(&table[i].baud);
    if (baud == speed)
    {
      _rx_delay_centering = pgm_read_word(&table[i].rx_delay_centering);
      _rx_delay_intrabit = pgm_read_word(&table[i].rx_delay_intrabit);
      _rx_delay_stopbit = pgm_read_word(&table[i].rx_delay_stopbit);
      _tx_delay = pgm_read_word(&table[i].tx_delay);
      break;
    }
  }

  // Set up RX interrupts, but only if we have a valid RX baud rate
  if (_rx_delay_stopbit)
  {
    if (digitalPinToPCICR(_receivePin))
    {
      *digitalPinToPCICR(_receivePin) |= _BV(digitalPinToPCICRbit(_receivePin));
      *digitalPinToPCMSK(_receivePin) |= _BV(digitalPinToPCMSKbit(_receivePin));
    }
    tunedDelay(_tx_delay); // if we were low this establishes the end
  }

#if _DEBUG
  pinMode(_DEBUG_PIN1, OUTPUT);
  pinMode(_DEBUG_PIN2, OUTPUT);
#endif


}

void SoftSerial::end()
{
  if (digitalPinToPCMSK(_receivePin))
    *digitalPinToPCMSK(_receivePin) &= ~_BV(digitalPinToPCMSKbit(_receivePin));
}


