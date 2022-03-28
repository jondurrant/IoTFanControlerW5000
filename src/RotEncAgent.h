/*
 * RotEncAgent.h
 *
 *  Created on: 28 Mar 2022
 *      Author: jondurrant
 */

#ifndef ROTENCAGENT_H_
#define ROTENCAGENT_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "AgentInterface.h"
#include <stdint.h>
#include "hardware/gpio.h"
#include "pico/time.h"

#include "RotEncListener.h"

#define LONGPRESSMSEC 300
#define MINSHORTPRESSMSEC 10

#define POLLTIMEUS 2000


class RotEncAgent : public AgentInterface{
public:
	RotEncAgent(uint8_t pGP, uint8_t aGP, uint8_t bGP,
			uint8_t numTicks);
	virtual ~RotEncAgent();

	/***
	 *  create the vtask
	 *  */
	virtual  bool start(UBaseType_t priority = tskIDLE_PRIORITY);

	int8_t getPos();

	void setListener(RotEncListener *listener);

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

	void handleGPIO(uint gpio, uint32_t events);

	void handleShortPress();

	void handleLongPress();

	void handleRotate(bool clockwise);

	void poll();



private:
	static RotEncAgent * pSelf;
	static void gpioCallback (uint gpio, uint32_t events);

	RotEncListener * pListener = NULL;

	uint8_t xPushGP;
	uint8_t xAGP;
	uint8_t xBGP;
	uint8_t xNumTicks=0;
	bool xA = false;
	bool xB = false;

	uint32_t xPushTime=0;


	uint8_t xLast=0;
	int8_t xCount=0;

	int16_t xPos = 0;
	int16_t xDelta = 0;


	uint8_t xCW[4] ={2,0,3,1};
	uint8_t xCCW[4]={1,3,0,2};

};

#endif /* ROTENCAGENT_H_ */
