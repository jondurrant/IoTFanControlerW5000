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
#include "EthHelper.h"

enum RotEncEvent { REENone, REEShort, REELong, REECW, REECCW };
enum DisplayAgentState { DASCarosel, DASState, DASEdit };

class DisplayAgent : public AgentInterface, public RotEncListener {
public:
	DisplayAgent(OledDisplay *d, FanState *state, EthHelper *eth);
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


	void displayState(RotEncEvent event);

	void doEdit(RotEncEvent event, int16_t min, int16_t max, int16_t inc=1);

private:
	OledDisplay *pDisplay = NULL;
	EthHelper *pEth = NULL;

	uint8_t xSpeed = 0;
	float xTemp= 0.0;

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
