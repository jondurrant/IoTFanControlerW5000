/*
 * RotEncAgent.h
 *
 * Rotary Encoder Switch Management
 * Samples state at 2ms intervals
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

// Filters for Long and Short Press
#define LONGPRESSMSEC 300
#define MINSHORTPRESSMSEC 10

//Sampling time
#define POLLTIMEUS 2000


class RotEncAgent : public AgentInterface{
public:
	/***
	 * Constructor
	 * @param pGP - GPIO pin for the push switch
	 * @param aGP - GPIO pin for RotEnc A
	 * @param bGP - GPIO pin for RotEnc B
	 * @param numTicks - numTicks in 360 rotation
	 */
	RotEncAgent(uint8_t pGP, uint8_t aGP, uint8_t bGP,
			uint8_t numTicks);

	/***
	 * Destructor
	 */
	virtual ~RotEncAgent();

	/***
	 *  create the vtask
	 *  */
	virtual  bool start(UBaseType_t priority = tskIDLE_PRIORITY);

	/***
	 * Get current position of RotEnv
	 * @return
	 */
	int8_t getPos();

	/***
	 * Set a single listener to RotEnv events
	 * @param listener
	 */
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

	/***
	 * handle GPIO push switch events
	 * @param gpio - GPIO number
	 * @param events - Event
	 */
	void handleGPIO(uint gpio, uint32_t events);

	/***
	 * Process short push
	 */
	void handleShortPress();

	/***
	 * Process long press
	 */
	void handleLongPress();

	/***
	 * Handle rotate
	 * @param clockwise - true if clockwise, otherwise counter
	 */
	void handleRotate(bool clockwise);

	/***
	 * poll the RotEnv status
	 */
	void poll();



private:
	//Used for call back functions to find the object
	static RotEncAgent * pSelf;

	/***
	 * Call back function for push switch
	 * @param gpio
	 * @param events
	 */
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


	//These are lookups for the sequence to see in CW and CCW motion
	uint8_t xCW[4] ={2,0,3,1};
	uint8_t xCCW[4]={1,3,0,2};

	//Cache event briefly as we can't push events within intrupt
	bool xShort = false;
	bool xLong = false;

};

#endif /* ROTENCAGENT_H_ */
