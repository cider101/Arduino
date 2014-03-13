/*
 * SerialBase.cpp
 *
 *  Created on: 13.03.2014
 *      Author: REWEL
 */

#include "SerialBase.h"

inline void SerialBase::flush(Direction direction) {
	switch(direction) {
		case Rx : {
			size_t av = available();
			for(size_t i = 0; i < av; i++) {
				read();
			} }
			break;

		case Tx :
			//flush output
			break;

		case RxTx :
			//flush input & output
			break;
		case None : /*ignore*/ break;
	}
}


