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

class RGBLEDMgr : public  MQTTAgentObserver {
public:
	RGBLEDMgr(RGBLEDAgent * led);
	virtual ~RGBLEDMgr();

	virtual void MQTTOffline();

	virtual void MQTTOnline();

	virtual void MQTTSend();

	virtual void MQTTRecv();

private:
	RGBLEDAgent * pLed = NULL;
};

#endif /* RGBLEDMGR_H_ */
