/*
 * SerialBase.h
 *
 *  Created on: 13.03.2014
 *  Author: cider101
 */

#ifndef SERIALBASE_H_
#define SERIALBASE_H_

#include "Stream.h"

struct SerialBase : public Stream {
		enum Direction {None=0, Rx=1, Tx=2, RxTx=3};

		virtual void begin(unsigned long) = 0;
	   virtual int peek(size_t pos) = 0;
	   virtual int peek() { return peek(0); }

	   virtual void flush(Direction direction) = 0;
	   virtual void flush() { flush(Tx); }

	   virtual size_t available(Direction direction);
	   using Stream::available;

	   bool hasOverflow(Direction dir) {
	   	bool val = _overflow_flags & dir;
	   	_overflow_flags &= ~dir;
	   	return val;
	   }

//	   virtual bool hasUnderflow() = 0;

	protected:
		uint8_t _overflow_flags;

};

template<bool> struct IndexType {};
template<> struct IndexType<true> { typedef uint8_t index_type; };
template<> struct IndexType<false> { typedef size_t index_type; };

template<size_t rx_buffer_size>
class RxBufferedSerialBase : public SerialBase {
	public:
		enum {RXSIZE = rx_buffer_size };

		int read()
		{ return _rx_buffer_head == _rx_buffer_tail ? -1 : get(); }

		int peek(size_t pos)
		{ return (int)pos < available() ? _rx_buffer[(_rx_buffer_tail+pos)%RXSIZE] : -1; }

		void flush(Direction dir) {
			if(dir & Rx) {
				_rx_buffer_head = _rx_buffer_tail;
				_overflow_flags &= ~Rx;
			}
		}

		size_t available(SerialBase::Direction direction)
		{ return direction & Rx ? rxAvailable() : 0; }

		int available(void)
		{ return rxAvailable(); }

		inline size_t rxAvailable(void)
		{ return (unsigned int)(RXSIZE + _rx_buffer_head - _rx_buffer_tail) % RXSIZE; }

	protected:
		int get() {
		    unsigned char c = _rx_buffer[_rx_buffer_tail];
		    _rx_buffer_tail = (uint8_t)(_rx_buffer_tail + 1) % RXSIZE;
		    return c;
		}

		bool put(const char c) {
		    uint8_t next = (unsigned int)(_rx_buffer_head + 1) % RXSIZE;
		    if(next == _rx_buffer_tail) {
		   	 _overflow_flags |= Rx;

		    } else {
		   	 _rx_buffer[_rx_buffer_head] = c;
		   	 _rx_buffer_head = next;
		    }
		    return _overflow_flags & Rx;
		}

	protected:
		RxBufferedSerialBase() : _rx_buffer_head(0), _rx_buffer_tail(0) {}

		uint8_t _overflow_flags;

		volatile typename IndexType<RXSIZE<256>::index_type _rx_buffer_head,  _rx_buffer_tail;
	   uint8_t _rx_buffer[RXSIZE];

};

template<size_t rx_buffer_size, size_t tx_buffer_size>
class RxTxBufferedSerialBase : public RxBufferedSerialBase<rx_buffer_size> {
	public:
		typedef RxBufferedSerialBase<rx_buffer_size> super;
		enum { TXSIZE = tx_buffer_size };

		size_t available(SerialBase::Direction direction)
		{ return direction & SerialBase::Tx ?  txAvailable() : super::available(SerialBase::Rx); }

	   using Stream::available;

		void flush(SerialBase::Direction dir) {
			super::flush(dir);
			if(dir & SerialBase::Tx) {
				_tx_buffer_head = _tx_buffer_tail;
			}
			SerialBase::_overflow_flags &= ~dir;
		}

		size_t txAvailable()
		{ return (unsigned int)(TXSIZE + _tx_buffer_head - _tx_buffer_tail) % TXSIZE; }

	protected:
		RxTxBufferedSerialBase() : _tx_buffer_head(0), _tx_buffer_tail(0) {}


	protected:
	   volatile typename IndexType<TXSIZE<256>::index_type _tx_buffer_head,  _tx_buffer_tail;
	   uint8_t _tx_buffer[TXSIZE];
};

#endif /* SERIALBASE_H_ */
