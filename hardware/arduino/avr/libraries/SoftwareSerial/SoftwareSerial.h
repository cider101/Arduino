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
#include <SerialBase.h>

/******************************************************************************
* Definitions
******************************************************************************/

#define _SS_MAX_RX_BUFF 64 // RX buffer size
#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

class SoftSerial : public BufferedSerialBase<_SS_MAX_RX_BUFF, 0>
{
	public:
	  // public methods
		SoftSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false);
	  ~SoftSerial();

	  void begin(unsigned long speed);
	  void end();

	  virtual size_t write(uint8_t byte);
	  using Print::write;

	protected:
	  typedef BufferedSerialBase<_SS_MAX_RX_BUFF, 0> super;
	  void recv();


private:
  uint8_t _receivePin;
  uint8_t _receiveBitMask;
  volatile uint8_t *_receivePortRegister;

  uint8_t _transmitBitMask;
  volatile uint8_t *_transmitPortRegister;

  uint16_t _rx_delay_centering;
  uint16_t _rx_delay_intrabit;
  uint16_t _rx_delay_stopbit;
  uint16_t _tx_delay;

  uint16_t _inverse_logic:1;


  // private methods
  uint8_t rx_pin_read();
  void tx_pin_low();
  void tx_pin_high();

  void setTX(uint8_t transmitPin);
  void setRX(uint8_t receivePin);

  // private static method for timing
  static inline void tunedDelay(uint16_t delay);


public:
  // public only for easy access by interrupt handlers
  static inline void handle_interrupt();
};


class SoftwareSerial : public SoftSerial {
	private:
		static SoftwareSerial *active_object;

	public:
		SoftwareSerial(uint8_t rx, uint8_t tx) : SoftSerial(rx, tx) {}

		void begin(unsigned long speed) { SoftSerial::begin(speed); listen(); }
		bool isListening() { return this == active_object; }
		bool overflow() { return hasOverflow(SerialBase::Rx); }
		bool listen();

		virtual int read() { return isListening() ? super::read() : -1; }
		virtual int available() { return isListening() ? super::available() : 0; }
		virtual int peek()  { return isListening() ? super::peek(0) : 0; }
		virtual void flush() { if (isListening()) { super::flush(SerialBase::Rx); }}

	/*private:*/
		static inline void handle_interrupt();
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
