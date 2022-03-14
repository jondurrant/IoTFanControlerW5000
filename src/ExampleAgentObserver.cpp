/*
 * ExampleAgentObserver.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: jondurrant
 */

#include "ExampleAgentObserver.h"
#include "MQTTConfig.h"

#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

ExampleAgentObserver::ExampleAgentObserver() {
	// TODO Auto-generated constructor stub

}

ExampleAgentObserver::~ExampleAgentObserver() {
	// TODO Auto-generated destructor stub
}

void ExampleAgentObserver::MQTTOffline(){

	LogInfo(("Offline"));
}

void ExampleAgentObserver::MQTTOnline(){
	LogInfo(("Online"));
}

void ExampleAgentObserver::MQTTSend(){
	LogInfo(("Send"));
}

void ExampleAgentObserver::MQTTRecv(){
	datetime_t t;
	rtc_get_datetime(&t);
	LogInfo(("Recv at %d-%d-%d %d:%d:%d",
			t.year,
			t.month,
			t.day,
			t.hour,
			t.min,
			t.sec
			));
}
