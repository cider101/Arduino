/*
SoftSerial.cpp (formerly NewSoftSerial.cpp) -
Multi-instance software serial library for Arduino/Wiring
-- Interrupt-driven receive and other improvements by ladyada
   (http://ladyada.net)
-- Tuning, circular buffer, derivation from class Print/Stream,
   multi-instance support, porting to 8MHz processors,
   various optimizations, PROGMEM delay tables, inverse logic and 
   direct port writing by Mikal Hart (http://www.arduiniana.org)
-- Pin change interrupt macros by Paul Stoffregen (http://www.pjrc.com)
-- 20MHz processor support by Garrett Mace (http://www.macetech.com)
-- ATmega1280/2560 support by Brett Hagman (http://www.roguerobotics.com/)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The latest version of this library can always be found at
http://arduiniana.org.
*/

#include "SoftwareSerial.h"
#include <Arduino.h>

//
// Statics
//
SoftwareSerial *SoftwareSerial::active_object = 0;

inline void SoftwareSerial::handle_interrupt() {
	SoftSerial::handle_interrupt(&_sharedInstance);
}

// This function sets the current object as the "listening"
// one and returns true if it replaces another
bool SoftwareSerial::listen() {

	if (active_object == this) {
		return false;
	}


	uint8_t oldSREG = SREG;
	cli();
	_sharedInstance.flush(SerialBase::Rx);
	_sharedInstance.configure(_rx, _tx, _inverse_logic);
	active_object = this;
	SREG = oldSREG;

	return true;
}
