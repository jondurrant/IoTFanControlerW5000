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
	RotEncEvent event = REENone;

    for( ;; )
    {
    	screen++;

    	if( xQueueReceive( xRotEnc, (void *) &event ,( TickType_t )0) == pdPASS ){
    		if (xDAState == DASCarosel){
    			xDAState = DASState;
    		}
    		if (xDAState == DASState){
				if (event == REECW){
					xStateItem++;
					if (xStateItem > 4)
						xStateItem = 0;
					event = REENone;
				}
				if (event == REECCW){
					xStateItem--;
					if (xStateItem <0)
						xStateItem = 4;
					event = REENone;
				}
				if (event == REEShort){
					xDAState = DASEdit;
					event = REENone;
				}
    		}
    	    screen = 80;
    	} else {
    		event = REENone;
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
    		case 81:
    		case 82:
    		case 83:
    		case 84:
    			//pDisplay->displayString("UI","TODO", 2);
    			displayState(event);
    			break;
    	}
    	if (screen > 200){
    		screen = 0;
    		xDAState = DASCarosel;
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

void DisplayAgent::displayState(RotEncEvent event){

	LogInfo(("event %d xDAState %d  xStateItem %d", event, xDAState, xStateItem));

	if (pState == NULL){
		pDisplay->displayString("No","State", 2);
	} else {
		if (xDAState == DASState){
			switch(xStateItem){
			case 0: //Fan Speed
				xEditValue = pState->getCurrentSpeed();
				sprintf(xBuf1, "%d%%", xEditValue);
				pDisplay->displayString("Fan",xBuf1, 2);
				break;
			case 1: //EnvTemp
				sprintf(xBuf1, "%.2fC", pState->getEnvTemp());
				pDisplay->displayString("Env Temp",xBuf1, 2);
				break;
			case 2: //Pre 1
				xEditValue = pState->getPreTemp()[0];
				sprintf(xBuf1, "%dC %d%%", xEditValue, pState->getPreSpeed()[0]);
				pDisplay->displayString("Pre1",xBuf1, 2);
				break;
			case 3: //Pre 2
				xEditValue = pState->getPreTemp()[1];
				sprintf(xBuf1, "%dC %d%%", xEditValue, pState->getPreSpeed()[1]);
				pDisplay->displayString("Pre2",xBuf1, 2);
				break;
			case 4: //Pre 3
				xEditValue = pState->getPreTemp()[2];
				sprintf(xBuf1, "%dC %d%%", xEditValue, pState->getPreSpeed()[2]);
				pDisplay->displayString("Pre3",xBuf1, 2);
				break;
			default:
				pDisplay->displayString("Unknown","State", 2);
			}
		} else {

			switch(xStateItem){
			case 0: //Fan Speed
				doEdit(event, 0, 100);
				sprintf(xBuf1, "%d%%", xEditValue);
				pDisplay->displayString("Edit Fan",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setCurrentSpeed(xEditValue);
					xDAState = DASState;
					event = REENone;
				}
				break;
			case 2: //Pre 1 Temp
				doEdit(event, 10, 40);
				sprintf(xBuf1, "%dC %d%%", xEditValue, pState->getPreSpeed()[0]);
				pDisplay->displayString("Edit Temp",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setPreTemp(xEditValue, 0);
					xStateItem += 100;
					event = REENone;
					xEditValue = pState->getPreSpeed()[0];
				}
				break;
			case 102: //Pre 1 Temp
				doEdit(event, 0, 100);
				sprintf(xBuf1, "%dC %d%%", pState->getPreTemp()[0], xEditValue);
				pDisplay->displayString("Edit Speed",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setPreSpeed(xEditValue, 0);
					xDAState = DASState;
					xStateItem -= 100;
					event = REENone;
				}
				break;
			case 3: //Pre 2 Temp
				doEdit(event, 10, 40);
				sprintf(xBuf1, "%dC %d%%", xEditValue, pState->getPreSpeed()[1]);
				pDisplay->displayString("Edit Temp",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setPreTemp(xEditValue, 1);
					xStateItem += 100;
					event = REENone;
					xEditValue = pState->getPreSpeed()[1];
				}
				break;
			case 103: //Pre 2 Temp
				doEdit(event, 0, 100);
				sprintf(xBuf1, "%dC %d%%", pState->getPreTemp()[1], xEditValue);
				pDisplay->displayString("Edit Speed",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setPreSpeed(xEditValue, 1);
					xDAState = DASState;
					xStateItem -= 100;
					event = REENone;
				}
				break;
			case 4: //Pre 3 Temp
				doEdit(event, 10, 40);
				sprintf(xBuf1, "%dC %d%%", xEditValue, pState->getPreSpeed()[2]);
				pDisplay->displayString("Edit Temp",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setPreTemp(xEditValue, 2);
					xStateItem += 100;
					event = REENone;
					xEditValue = pState->getPreSpeed()[2];
				}
				break;
			case 104: //Pre 1 Temp
				doEdit(event, 0, 100);
				sprintf(xBuf1, "%dC %d%%", pState->getPreTemp()[2], xEditValue);
				pDisplay->displayString("Edit Speed",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setPreSpeed(xEditValue, 2);
					xDAState = DASState;
					xStateItem -= 100;
					event = REENone;
				}
				break;
			default:
				xDAState = DASState;
			}
		}
	}
}

void DisplayAgent::doEdit(RotEncEvent event, int16_t min, int16_t max){
	if (event == REECW){
		xEditValue ++;
		if (xEditValue > max){
			xEditValue = min;
		}
	}
	if (event == REECCW){
		xEditValue --;
		if (xEditValue < min){
			xEditValue = max;
		}
	}
}

