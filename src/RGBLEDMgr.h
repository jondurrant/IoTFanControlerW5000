/*
 * RGBLEDMgr.h
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
	RGBLEDMgr(RGBLEDAgent * led);
	virtual ~RGBLEDMgr();

	virtual void MQTTOffline();

	virtual void MQTTOnline();

	virtual void MQTTSend();

	virtual void MQTTRecv();

	void setDisplayAgent(DisplayAgent *disp);

private:
	RGBLEDAgent * pLed = NULL;
	DisplayAgent * pDisp = NULL;
};

#endif /* RGBLEDMGR_H_ */
