/*
 * interrupt.cpp
 *
 *  Created on: 14.03.2014
 *  Author: cider101
 */

#include <Arduino.h>

#include "SoftSerial.h"
#include "SoftwareSerial.h"

inline bool SoftSerial::handle_interrupt(SoftSerial *serial) {
	if (serial == NULL) {
		return false;
	}

	// If RX line is high, then we don't see any start bit
	// so interrupt is probably not for us
	bool isHigh = serial->rx_pin_read();
	if (serial->_inverse_logic ? !isHigh : isHigh) {
		return false;
	}

	serial->recv();
	return true;
}

inline void SoftwareSerial::handle_interrupt() {
	SoftSerial::handle_interrupt(&_sharedInstance);
}

#define MAX_SOFT_SERIALS 8 //TODO use original definition of MAX_SOFT_SERIALS
extern SoftSerial* instances[MAX_SOFT_SERIALS];

inline void SoftSerial::handle_interrupt()
{
	for(int i = 0; i < MAX_SOFT_SERIALS; ++i) {
		handle_interrupt(instances[i]);
	}
}

#if defined(PCINT0_vect)
ISR(PCINT0_vect)
{
	SoftwareSerial::handle_interrupt();
	SoftSerial::handle_interrupt();
}
#endif

#if defined(PCINT1_vect)
ISR(PCINT1_vect)
{
	SoftwareSerial::handle_interrupt();
	SoftSerial::handle_interrupt();
}
#endif

#if defined(PCINT2_vect)
ISR(PCINT2_vect)
{
	SoftwareSerial::handle_interrupt();
	SoftSerial::handle_interrupt();
}
#endif

#if defined(PCINT3_vect)
ISR(PCINT3_vect)
{
	SoftwareSerial::handle_interrupt();
	SoftSerial::handle_interrupt();
}
#endif
