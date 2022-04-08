/*
 * DisplayAgent.h
 *
 * Agent responsible for managing the OLED Display and triggering the RGB LED
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
#include "EthHelper.h"


// Type of RotEnc input
enum RotEncEvent { REENone, REEShort, REELong, REECW, REECCW };

// Display can be in Carosel mode displaying sceens for time interval
// or showing state under control or in Edit of a state item
enum DisplayAgentState { DASCarosel, DASState, DASEdit };

class DisplayAgent : public AgentInterface, public RotEncListener {
public:
	/***
	 * Constructor
	 * @param d - OledDisplay that will talk to the hardware
	 * @param state - State object
	 * @param eth = Ethernet helper
	 */
	DisplayAgent(OledDisplay *d, FanState *state, EthHelper *eth);

	/***
	 * Destructor
	 */
	virtual ~DisplayAgent();


	/***
	 * Creat the task
	 * @param priority
	 * @return true if sucessfull
	 */
	virtual  bool start(UBaseType_t priority = tskIDLE_PRIORITY);


	/***
	 * Show ip address
	 * @param ip uint8_t[4]
	 */
	void showIP(uint8_t *ip);

	/***
	 * Show No IP address is currently available
	 */
	void noIP();

	/***
	 * RotEnc has had a short press event
	 * @param rotEnv
	 */
	virtual void shortPress(void * rotEnv);

	/***
	 * RotEnv has had a long press event
	 * @param rotEnv
	 */
	virtual void longPress(void * rotEnv);

	/***
	 * RotEnc has had a turn event
	 * @param clockwise
	 * @param pos
	 * @param rotEnc
	 */
	virtual void rotate(bool clockwise, int16_t pos, void * rotEnc);

	/***
	 * MQTT session is online or offline
	 * @param b - true if online
	 */
	void online(bool b);

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
	 * Handle the display of state or edit of state
	 * @param event
	 */
	void displayState(RotEncEvent event);

	/***
	 * Handle an edit request for a state item(xStateItem)
	 * @param event - type of RotEnv event
	 * @param min - minimum value of state item (xStateItem)
	 * @param max - maximum value of state item
	 * @param inc - increment on each turn, default is 1
	 */
	void doEdit(RotEncEvent event, int16_t min, int16_t max, int16_t inc=1);

private:
	OledDisplay *pDisplay = NULL;
	EthHelper *pEth = NULL;


	uint8_t xIP[4];

	QueueHandle_t xRotEnc;
	FanState *pState = NULL;

	int8_t xStateItem = 0;
	char xBuf1[40];
	char xBuf2[40];

	DisplayAgentState xDAState = DASCarosel;
	int16_t xEditValue = 0;
	bool xOnline=false;

};

#endif /* DISPLAYAGENT_H_ */
