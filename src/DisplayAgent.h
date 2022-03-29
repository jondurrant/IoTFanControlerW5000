/*
 * DisplayAgent.h
 *
 *  Created on: 16 Mar 2022
 *      Author: jondurrant
 */

#ifndef DISPLAYAGENT_H_
#define DISPLAYAGENT_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "AgentInterface.h"
#include "OledDisplay.h"
#include "RotEncListener.h"
#include "FanState.h"


class DisplayAgent : public AgentInterface, public RotEncListener {
public:
	DisplayAgent(OledDisplay *d, FanState *state);
	virtual ~DisplayAgent();


	/***
	 *  create the vtask
	 *  */
	virtual  bool start(UBaseType_t priority = tskIDLE_PRIORITY);


	void showTemp(float temp, uint8_t speed);


	void showIP(uint8_t *ip);

	void noIP();

	virtual void shortPress(void * rotEnv);

	virtual void longPress(void * rotEnv);

	virtual void rotate(bool clockwise, int16_t pos, void * rotEnc);

protected:
	/***
	 * Internal function to run the task from within the object
	 */
	void run();

	/***
	 * Internal function used by FreeRTOS to run the task
	 * @param pvParameters
	 */
	static void vTask( void * pvParameters );


	void displayState();

private:
	OledDisplay *pDisplay = NULL;


	uint8_t xSpeed = 0;
	float xTemp= 0.0;

	uint8_t xIP[4];

	QueueHandle_t xRotEnc;
	FanState *pState = NULL;

	int8_t xStateItem = 0;
	char xBuf1[40];
	char xBuf2[40];
};

#endif /* DISPLAYAGENT_H_ */
