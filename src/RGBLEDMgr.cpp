/*
 * RGBLEDMgr.cpp
 *
 *  Created on: 28 Nov 2021
 *      Author: jondurrant
 */

#include "RGBLEDMgr.h"
#include <stdlib.h>
#include "MQTTConfig.h"

RGBLEDMgr::RGBLEDMgr(RGBLEDAgent * led) {
	pLed = led;
}

RGBLEDMgr::~RGBLEDMgr() {
	// NOP
}

void RGBLEDMgr::MQTTOffline(){
	if (pLed != NULL){

		if (!pLed->set(RGBModeOn, 0xF5, 0xCE, 0x42)){
			LogError(("Set failed"));
		}
	}

	LogDebug(("Offline"));
}

void RGBLEDMgr::MQTTOnline(){
	if (pLed != NULL){
		if (pLed->set(RGBModeOn,0, 0, 0xFF)){
			LogError(("Set failed"));
		}
	}
	LogDebug(("Online"));
}

void RGBLEDMgr::MQTTSend(){
	if (pLed != NULL){
		if(!pLed->set(RGBModeOnce,0, 0, 0xFF)){
			LogError(("Set failed"));
		}
	}
	LogDebug(("Send"));
}

void RGBLEDMgr::MQTTRecv(){
	if (pLed != NULL){
		if(!pLed->set(RGBModeOnce,0, 0, 0xFF)){
			LogError(("Set failed"));
		}
	}
	LogDebug(("Recv"));
}
