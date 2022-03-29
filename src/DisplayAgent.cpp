/*
 * DisplayAgent.cpp
 *
 *  Created on: 16 Mar 2022
 *      Author: jondurrant
 */

#include "DisplayAgent.h"
#include <stdio.h>
#include <string.h>

#include "FanLogging.h"
#include "hardware/rtc.h"

enum RotEncEvent { REEShort, REELong, REECW, REECCW };

DisplayAgent::DisplayAgent(OledDisplay *d, FanState *state) {
	pDisplay = d;
	pState = state;
	noIP();
}

DisplayAgent::~DisplayAgent() {
	// TODO Auto-generated destructor stub
}

bool DisplayAgent::start(UBaseType_t priority){
	BaseType_t xReturned;

	xRotEnc = xQueueCreate( 10, sizeof( uint8_t ) );
	if (xRotEnc == NULL){
		LogError(("Unable to create queue"));
	} else {

		xReturned = xTaskCreate(
			DisplayAgent::vTask,       /* Function that implements the task. */
			"DisplayAgent",   /* Text name for the task. */
			500,             /* Stack size in words, not bytes. */
			( void * ) this,    /* Parameter passed into the task. */
			priority,/* Priority at which the task is created. */
			&xHandle
		);
	}
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
	char min[3];
	datetime_t t;
	RotEncEvent event;

    for( ;; )
    {
    	screen++;

    	if( xQueueReceive( xRotEnc, (void *) &event ,( TickType_t )0) == pdPASS ){
    		if (event == REECW){
    			xStateItem++;
    			if (xStateItem > 4)
    				xStateItem = 0;
    		}
    		if (event == REECCW){
				xStateItem--;
				if (xStateItem <0)
					xStateItem = 4;
			}
    	    screen = 80;
    	}


    	switch(screen){
    		case 10:
			    rtc_get_datetime(&t);
			    if (t.min < 10){
			    	sprintf(min, "0%d", t.min);
			    } else {
			    	sprintf(min, "%d", t.min);
			    }
			    if (t.sec < 10){
			    	sprintf(xBuf2, "0%d", t.sec);
			    } else {
			    	sprintf(xBuf2, "%d", t.sec);
			    }
			    sprintf(xBuf1,"%d:%s:%s",t.hour, min, xBuf2);
    			pDisplay->displayString("COOLER",xBuf1,2);
    			break;
    		case 20:
    		case 30:
    		case 40:
    			sprintf(xBuf1,"%.1fC", xTemp);
    			sprintf(xBuf2,"%d%%",  xSpeed);
    			pDisplay->displayString(xBuf1,xBuf2, 2);
    			break;
    		case 50:
    			if (xIP[0] == 0){
					pDisplay->displayString("NO IP","", 2);
    			} else {
    				sprintf(xBuf1,"%d.%d",xIP[0],xIP[1]);
    				sprintf(xBuf2,".%d.%d",xIP[2],xIP[3]);
					pDisplay->displayString(xBuf1,xBuf2, 2);
    			}
    		case 60:
    			screen = 0;
    			break;
    		case 80:
    			//pDisplay->displayString("UI","TODO", 2);
    			displayState();
    			break;
    	}
    	if (screen > 100){
    		screen = 0;
    	}
		vTaskDelay(100);
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

void DisplayAgent::shortPress(void * rotEnv){
	RotEncEvent event = REEShort;
	if( xRotEnc != NULL ){
		if( xQueueSendToBack( xRotEnc,( void * ) &event, ( TickType_t ) 10 ) != pdPASS ){
	           LogError(("Queue Full"));
		}
	}
}

void DisplayAgent::longPress(void * rotEnv){
	RotEncEvent event = REELong;
	if( xRotEnc != NULL ){
		if( xQueueSendToBack( xRotEnc,( void * ) &event, ( TickType_t ) 10 ) != pdPASS ){
			   LogError(("Queue Full"));
		}
	}
}

void DisplayAgent::rotate(bool clockwise, int16_t pos, void * rotEnc){
	RotEncEvent event = REECW;
	if (!clockwise){
		event = REECCW;
	}
	if( xRotEnc != NULL ){
		if( xQueueSendToBack( xRotEnc,( void * ) &event, ( TickType_t ) 10 ) != pdPASS ){
			   LogError(("Queue Full"));
		}
	}
}

void DisplayAgent::displayState(){

	LogInfo(("xStateItem %d", xStateItem));
	if (pState == NULL){
		pDisplay->displayString("No","State", 2);
	} else {
		switch(xStateItem){
		case 0: //Fan Speed
			sprintf(xBuf1, "%d%%", pState->getCurrentSpeed());
			pDisplay->displayString("Fan",xBuf1, 2);
			break;
		case 1: //EnvTemp
			sprintf(xBuf1, "%.2fC", pState->getEnvTemp());
			pDisplay->displayString("Env Temp",xBuf1, 2);
			break;
		case 2: //Pre 1
			sprintf(xBuf1, "%dC %d%%", pState->getPreTemp()[0], pState->getPreSpeed()[0]);
			pDisplay->displayString("Pre1",xBuf1, 2);
			break;
		case 3: //Pre 2
			sprintf(xBuf1, "%dC %d%%", pState->getPreTemp()[1], pState->getPreSpeed()[1]);
			pDisplay->displayString("Pre2",xBuf1, 2);
			break;
		case 4: //Pre 3
			sprintf(xBuf1, "%dC %d%%", pState->getPreTemp()[2], pState->getPreSpeed()[2]);
			pDisplay->displayString("Pre3",xBuf1, 2);
			break;
		default:
			pDisplay->displayString("Unknown","State", 2);
		}
	}
}

