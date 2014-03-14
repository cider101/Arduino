/*
SoftwareSerial.h (formerly NewSoftSerial.h) - 
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

#ifndef SoftwareSerial_h
#define SoftwareSerial_h

#include <inttypes.h>
#include "SoftSerial.h"

/******************************************************************************
* Definitions
******************************************************************************/

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif


class SoftwareSerial : public SerialBase {
	private:
		static SoftwareSerial *active_object;
		static SoftSerial _sharedInstance;

		uint8_t _rx:6, _tx:6;
		bool _inverse_logic:1;

	public:
		SoftwareSerial(uint8_t rx, uint8_t tx, bool inverse_logic = false)
		: _rx(rx), _tx(tx), _inverse_logic(inverse_logic) {}

		void begin(unsigned long speed) { listen(); _sharedInstance.begin(speed);  }
		bool isListening() { return this == active_object; }
		bool listen();

		bool overflow() { return hasOverflow(SerialBase::Rx); }
		virtual bool hasOverflow(SerialBase::Direction dir) { return isListening() ? _sharedInstance.hasOverflow(SerialBase::Rx) : false; }

		virtual int read() { return isListening() ? _sharedInstance.read() : -1; }
		virtual size_t write(uint8_t value) { return isListening() ? _sharedInstance.write(value) : 0; }
		virtual int available() { return isListening() ? _sharedInstance.available() : 0; }

		virtual int peek()  { return isListening() ? _sharedInstance.peek(0) : -1; }
		virtual int peek(size_t)  { return isListening() ? _sharedInstance.peek(0) : -1; }

		virtual void flush() { flush(SerialBase::Rx); }
		virtual void flush(SerialBase::Direction dir) { isListening() ? _sharedInstance.flush(dir) : nop(); }


	/*private:*/
		static inline void handle_interrupt();
	private:
		void nop() {};
};

//// Arduino 0012 workaround
//#undef int
//#undef char
//#undef long
//#undef byte
//#undef float
//#undef abs
//#undef round

#endif
