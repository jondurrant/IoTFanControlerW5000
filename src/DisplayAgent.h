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

class DisplayAgent : public AgentInterface{
public:
	DisplayAgent(OledDisplay *d);
	virtual ~DisplayAgent();


	/***
	 *  create the vtask
	 *  */
	virtual  bool start(UBaseType_t priority = tskIDLE_PRIORITY);


	void showTemp(float temp, uint8_t speed);


	void showIP(uint8_t *ip);

	void noIP();

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

private:
	OledDisplay *pDisplay = NULL;


	uint8_t xSpeed = 0;
	float xTemp= 0.0;

	uint8_t xIP[4];
};

#endif /* DISPLAYAGENT_H_ */
