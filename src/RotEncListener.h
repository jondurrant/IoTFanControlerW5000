/*
 * RotEnvListener.h
 *
 *  Created on: 24 Mar 2022
 *      Author: jondurrant
 */

#ifndef ROTENC_SRC_ROTENCLISTENER_H_
#define ROTENC_SRC_ROTENCLISTENER_H_

#include <stdint.h>

class RotEncListener {
public:
	RotEncListener();
	virtual ~RotEncListener();

	virtual void shortPress(void * rotEnv);

	virtual void longPress(void * rotEnv);

	virtual void rotate(bool clockwise, int16_t pos, void * rotEnc);

};

#endif /* ROTENC_SRC_ROTENCLISTENER_H_ */
