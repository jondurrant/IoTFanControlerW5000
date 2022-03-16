/*
 * DisplayAgent.cpp
 *
 *  Created on: 16 Mar 2022
 *      Author: jondurrant
 */

#include "DisplayAgent.h"
#include <stdio.h>
#include <string.h>


DisplayAgent::DisplayAgent(OledDisplay *d) {
	pDisplay = d;
	noIP();
}

DisplayAgent::~DisplayAgent() {
	// TODO Auto-generated destructor stub
}

bool DisplayAgent::start(UBaseType_t priority){
	BaseType_t xReturned;

	xReturned = xTaskCreate(
		DisplayAgent::vTask,       /* Function that implements the task. */
		"RGB",   /* Text name for the task. */
		100,             /* Stack size in words, not bytes. */
		( void * ) this,    /* Parameter passed into the task. */
		priority,/* Priority at which the task is created. */
		&xHandle
	);
	return (xReturned == pdPASS);
}


void DisplayAgent::vTask( void * pvParameters )
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below.
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    */
	DisplayAgent *task = (DisplayAgent *) pvParameters;
	task->run();
}

void DisplayAgent::run(){

	uint8_t screen = 0;
	char buf[80];
    for( ;; )
    {
    	screen++;
    	switch(screen){
    		case 1:
    			pDisplay->displayString("COOLER",2);
    			break;
    		case 2:
    		case 3:
    		case 4:
    			sprintf(buf,"%.1fC %d%%",xTemp, xSpeed);
    			pDisplay->displayString(buf, 1);
    			break;
    		case 5:
    			if (xIP[0] == 0){
					pDisplay->displayString("NO IP", 2);
    			} else {
    				sprintf(buf,"%d.%d.%d.%d",xIP[0],xIP[1],xIP[2],xIP[3]);
					pDisplay->displayString(buf, 1);
    			}
    		default:
    			screen=0;
    	}
		vTaskDelay(1000);
    }
}


void DisplayAgent::showTemp(float temp, uint8_t speed){
	xTemp = temp;
	xSpeed = speed;
}

void DisplayAgent::showIP(uint8_t *ip){
	memcpy(xIP, ip, 4);
}

void DisplayAgent::noIP(){
	memset(xIP, 0, 4);
}


