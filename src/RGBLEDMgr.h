/*
 * RGBLEDMgr.h
 *
 * Bridge status to the LED
 *
 *  Created on: 28 Nov 2021
 *      Author: jondurrant
 */


#ifndef RGBLEDMGR_H_
#define RGBLEDMGR_H_

#include <MQTTAgentObserver.h>
#include <RGBLEDAgent.h>
#include "DisplayAgent.h"

class RGBLEDMgr : public  MQTTAgentObserver {
public:
	/***
	 * Constructor
	 * @param led - Led Agent
	 */
	RGBLEDMgr(RGBLEDAgent * led);

	/***
	 * Destructor
	 */
	virtual ~RGBLEDMgr();

	/***
	 * Notify MQTT is offline
	 */
	virtual void MQTTOffline();

	/***
	 * Notify MQTT is Online
	 */
	virtual void MQTTOnline();

	/***
	 * Notify data sent
	 */
	virtual void MQTTSend();

	/***
	 * Notify data received
	 */
	virtual void MQTTRecv();

	/***
	 * Set the Display Agent
	 * Allows OLED display to also know status
	 * @param disp
	 */
	void setDisplayAgent(DisplayAgent *disp);

private:
	RGBLEDAgent * pLed = NULL;
	DisplayAgent * pDisp = NULL;
};

#endif /* RGBLEDMGR_H_ */
