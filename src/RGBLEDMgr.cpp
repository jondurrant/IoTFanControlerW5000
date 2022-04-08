/*
 * RGBLEDMgr.cpp
 *
 * Bridge status to the LED
 *
 *  Created on: 28 Nov 2021
 *      Author: jondurrant
 */

#include "RGBLEDMgr.h"
#include <stdlib.h>
#include "MQTTConfig.h"

/***
 * Constructor
 * @param led - Led Agent
 */
RGBLEDMgr::RGBLEDMgr(RGBLEDAgent * led) {
	pLed = led;
}

/***
 * Destructor
 */
RGBLEDMgr::~RGBLEDMgr() {
	// NOP
}

/***
 * Notify MQTT is offline
 */
void RGBLEDMgr::MQTTOffline(){
	if (pLed != NULL){

		if (!pLed->set(RGBModeOn, 0xF5, 0xCE, 0x42)){
			LogError(("Set failed"));
		}
	}

	if (pDisp != NULL){
		pDisp->online(false);
	}

	LogDebug(("Offline"));
}

/***
 * Notify MQTT is Online
 */
void RGBLEDMgr::MQTTOnline(){
	if (pLed != NULL){
		if (pLed->set(RGBModeOn,0, 0, 0xFF)){
			LogError(("Set failed"));
		}
	}

	if (pDisp != NULL){
		pDisp->online(true);
	}
	LogDebug(("Online"));
}


/***
 * Notify data sent
 */
void RGBLEDMgr::MQTTSend(){
	if (pLed != NULL){
		if(!pLed->set(RGBModeOnce,0, 0, 0xFF)){
			LogError(("Set failed"));
		}
	}
	LogDebug(("Send"));
}

/***
 * Notify data received
 */
void RGBLEDMgr::MQTTRecv(){
	if (pLed != NULL){
		if(!pLed->set(RGBModeOnce,0, 0, 0xFF)){
			LogError(("Set failed"));
		}
	}
	LogDebug(("Recv"));
}

/***
 * Set the Display Agent
 * Allows OLED display to also know status
 * @param disp
 */
void RGBLEDMgr::setDisplayAgent(DisplayAgent *disp){
	pDisp = disp;
}
