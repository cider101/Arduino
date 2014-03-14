/*
 * SoftSerial.h
 *
 *  Created on: 14.03.2014
 *  Author: cider101
 */

#ifndef SOFTSERIAL_H_
#define SOFTSERIAL_H_

#include <SerialBase.h>

#define _SS_MAX_RX_BUFF 64 // RX buffer size


class SoftSerial: public RxBufferedSerialBase<_SS_MAX_RX_BUFF> {
public:
		// public methods
		SoftSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false);
		~SoftSerial();

		void begin(unsigned long speed);
		void end();

		virtual size_t write(uint8_t byte);
		using Print::write;

	protected:
		typedef RxBufferedSerialBase<_SS_MAX_RX_BUFF> super;
		static inline void tunedDelay(uint16_t delay);
		void recv();

		uint8_t rx_pin_read();
		void tx_pin_write(uint8_t);

		void configure(uint8_t rx, uint8_t tx, bool inverse_logic);
		void setTX(uint8_t transmitPin);
		void setRX(uint8_t receivePin);

		friend class SoftwareSerial;
		static inline bool handle_interrupt(SoftSerial*);

	protected:
		uint8_t _receivePin:6;
		uint8_t _receiveBitMask;
		volatile uint8_t *_receivePortRegister;

		uint8_t _transmitBitMask;
		volatile uint8_t *_transmitPortRegister;

		uint16_t _rx_delay_centering;
		uint16_t _rx_delay_intrabit;
		uint16_t _rx_delay_stopbit;
		uint16_t _tx_delay;

		uint16_t _inverse_logic :1;
	public:
		// public only for easy access by interrupt handlers
		static inline void handle_interrupt();
};

#endif /* SOFTSERIAL_H_ */
