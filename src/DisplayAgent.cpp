/*
 * DisplayAgent.cpp
 *
 * Agent responsible for managing the OLED Display and triggering the RGB LED
 *
 *  Created on: 16 Mar 2022
 *      Author: jondurrant
 */

#include "DisplayAgent.h"
#include <stdio.h>
#include <string.h>

#include "FanLogging.h"
#include "hardware/rtc.h"


/***
 * Constructor
 * @param d - OledDisplay that will talk to the hardware
 * @param state - State object
 * @param eth = Ethernet helper
 */
DisplayAgent::DisplayAgent(OledDisplay *d, FanState *state, EthHelper *eth) {
	pDisplay = d;
	pState = state;
	pEth = eth;
	noIP();
}

/***
 * Destructor
 */
DisplayAgent::~DisplayAgent() {
	// NOP
}

/***
 * Creat the task
 * @param priority
 * @return true if sucessfull
 */
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
					if (xStateItem > 9)
						xStateItem = 0;
					event = REENone;
				}
				if (event == REECCW){
					xStateItem--;
					if (xStateItem <0)
						xStateItem = 9;
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
    			if (pState != NULL) {
					sprintf(xBuf1,"%.1fC", pState->getEnvTemp());
					sprintf(xBuf2,"%d%%",  pState->getCurrentSpeed());

    				if (pState->getOverrideMinutes() > 0){
    					sprintf(xBuf2,"%d%% OR",  pState->getCurrentSpeed());
    				}
    				pDisplay->displayString(xBuf1,xBuf2, 2);
    			} else {
    				pDisplay->displayString("Missing","STATE", 2);
    			}

    			break;
    		case 50:
    			if (xIP[0] == 0){
					pDisplay->displayString("NO IP","", 2);
    			} else {
    				if (xOnline){
    					pDisplay->displayString("Online","", 2);
    				} else {
    					pDisplay->displayString("Offline","", 2);
    				}
    			}
    			break;
    		case 60:
    			screen = 0;
    			break;
    	}
    	if (screen >= 80){
    		displayState(event);
    	}

    	if (screen > 200){
    		screen = 0;
    		xDAState = DASCarosel;
    	}
		vTaskDelay(100);
    }
}


/***
 * Show ip address
 * @param ip uint8_t[4]
 */
void DisplayAgent::showIP(uint8_t *ip){
	memcpy(xIP, ip, 4);
}

/***
 * Show No IP address is currently available
 */
void DisplayAgent::noIP(){
	memset(xIP, 0, 4);
}

/***
 * RotEnc has had a short press event
 * @param rotEnv
 */
void DisplayAgent::shortPress(void * rotEnv){
	RotEncEvent event = REEShort;
	if( xRotEnc != NULL ){
		if( xQueueSendToBack( xRotEnc,( void * ) &event, ( TickType_t ) 10 ) != pdPASS ){
	           LogError(("Queue Full"));
		}
	}
}

/***
 * RotEnv has had a long press event
 * @param rotEnv
 */
void DisplayAgent::longPress(void * rotEnv){
	RotEncEvent event = REELong;
	if( xRotEnc != NULL ){
		if( xQueueSendToBack( xRotEnc,( void * ) &event, ( TickType_t ) 10 ) != pdPASS ){
			   LogError(("Queue Full"));
		}
	}
}

/***
 * RotEnc has had a turn event
 * @param clockwise
 * @param pos
 * @param rotEnc
 */
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

/***
 * Handle the display of state or edit of state
 * @param event
 */
void DisplayAgent::displayState(RotEncEvent event){
	datetime_t t;
	char min[3];
	uint16_t om;

	//LogInfo(("event %d xDAState %d  xStateItem %d", event, xDAState, xStateItem));

	if (pState == NULL){
		pDisplay->displayString("No","State", 2);
	} else {
		if (xDAState == DASState){
			switch(xStateItem){
			case 0: //EnvTemp
				sprintf(xBuf1, "%.2fC", pState->getEnvTemp());
				pDisplay->displayString("Env Temp",xBuf1, 2);
				break;
			case 1: //Fan Speed
				xEditValue = pState->getCurrentSpeed();
				sprintf(xBuf1, "Fan %d%%", xEditValue);
				om = pState->getOverrideMinutes();
				sprintf(xBuf2, "%d %d:%d", om / (60*24),  (om/60)%24, om%60);
				pDisplay->displayString(xBuf1,xBuf2, 2);
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
			case 5: //Max Night Speed
				xEditValue = pState->getMaxNightSpeed();
				sprintf(xBuf1, "%d%%", xEditValue);
				pDisplay->displayString("MaxNight",xBuf1, 2);
				break;
			case 6: //Day start
				xEditValue = pState->getDayStart();
				sprintf(xBuf1, "%d:00", xEditValue);
				pDisplay->displayString("DayStart",xBuf1, 2);
				break;
			case 7: //Day start
				xEditValue = pState->getDayEnd();
				sprintf(xBuf1, "%d:00", xEditValue);
				pDisplay->displayString("Day End",xBuf1, 2);
				break;
			case 8: // Clock
				rtc_get_datetime(&t);
				if (t.min < 10){
					sprintf(min, "0%d", t.min);
				} else {
					sprintf(min, "%d", t.min);
				}
				if (t.sec < 10){
					sprintf(xBuf1, "0%d", t.sec);
				} else {
					sprintf(xBuf1, "%d", t.sec);
				}
				sprintf(xBuf2, "%d:%s:%s",t.hour, min, xBuf1);
				sprintf(xBuf1, "%d-%d-%d", t.year%1000, t.month, t.day);
				pDisplay->displayString(xBuf1,xBuf2,2);
				break;
			case 9: // IP Address
				if (xIP[0] == 0){
					pDisplay->displayString("NO IP","", 2);
				} else {
					sprintf(xBuf1,"IP%d%d",xIP[0],xIP[1]);
					sprintf(xBuf2,".%d.%d",xIP[2],xIP[3]);
					pDisplay->displayString(xBuf1,xBuf2, 2);
				}
				break;
			default:
				pDisplay->displayString("Unknown","State", 2);
			}
		} else {

			switch(xStateItem){
			case 1: //Fan Speed
				doEdit(event, 0, 100);
				sprintf(xBuf1, "%d%%", xEditValue);
				pDisplay->displayString("Edit Fan",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					//If no override in place then protect fan speed while we set override
					if (pState->getOverrideMinutes() < 1){
						pState->setOverrideMinutes(10);
					}
					pState->setCurrentSpeed(xEditValue);
					pState->setOn(xEditValue > 0);
					LogInfo(("Fan Speed Override %d = %d",xEditValue, pState->getCurrentSpeed()));
					xStateItem += 100;
					event = REENone;
					xEditValue = pState->getOverrideMinutes();
					if (xEditValue == 0){
						xEditValue = 10;
					}
				}
				break;
			case 101:
				doEdit(event, 0, 24*60*10, 10);
				sprintf(xBuf1, "%d %d:%d", xEditValue / (60*24),  (xEditValue/60)%24, xEditValue%60);
				pDisplay->displayString("Edit Fan",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setOverrideMinutes(xEditValue);
					xStateItem -= 100;
					event = REENone;
					xDAState = DASState;
				}
				break;
			case 2: //Pre 1 Temp
				doEdit(event, 10, 40);
				sprintf(xBuf1, "%dC %d%%", xEditValue, pState->getPreSpeed()[0]);
				pDisplay->displayString("Ed Temp",xBuf1, 2);
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
				pDisplay->displayString("Ed Speed",xBuf1, 2);
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
				pDisplay->displayString("Ed Temp",xBuf1, 2);
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
				pDisplay->displayString("Ed Speed",xBuf1, 2);
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
				pDisplay->displayString("Ed Temp",xBuf1, 2);
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
				pDisplay->displayString("Ed Speed",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setPreSpeed(xEditValue, 2);
					xDAState = DASState;
					xStateItem -= 100;
					event = REENone;
				}
				break;
			case 5: //Max Night Speed
				doEdit(event, 0, 100);
				sprintf(xBuf1, "%d%%", xEditValue);
				pDisplay->displayString("Edit Max",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setMaxNightSpeed(xEditValue);
					xDAState = DASState;
					event = REENone;
				}
				break;
			case 6: //Day start
				doEdit(event, 0, 23);
				sprintf(xBuf1, "%d:00", xEditValue);
				pDisplay->displayString("Ed Start",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setDayStart(xEditValue);
					xDAState = DASState;
					event = REENone;
				}
				break;
			case 7: //Day start
				doEdit(event, 0, 23);
				sprintf(xBuf1, "%d:00", xEditValue);
				pDisplay->displayString("Edit End",xBuf1, 2);
				if ((event == REEShort) || (event == REELong)){
					pState->setDayEnd(xEditValue);
					xDAState = DASState;
					event = REENone;
				}
				break;
			case 8: // Clock
				rtc_get_datetime(&t);
				if (t.min < 10){
					sprintf(min, "0%d", t.min);
				} else {
					sprintf(min, "%d", t.min);
				}
				if (t.sec < 10){
					sprintf(xBuf1, "0%d", t.sec);
				} else {
					sprintf(xBuf1, "%d", t.sec);
				}
				sprintf(xBuf2, "%d:%s:%s",t.hour, min, xBuf1);
				pDisplay->displayString("NTP Req", xBuf2,2);
				//pEth->syncRTCwithSNTP();
				xDAState = DASState;
				event = REENone;
				break;
			default:
				xDAState = DASState;
			}
		}
	}
}

/***
 * Handle an edit request for a state item(xStateItem)
 * @param event - type of RotEnv event
 * @param min - minimum value of state item (xStateItem)
 * @param max - maximum value of state item
 * @param inc - increment on each turn, default is 1
 */
void DisplayAgent::doEdit(RotEncEvent event, int16_t min, int16_t max, int16_t inc){
	if (event == REECW){
		xEditValue += inc;
		if (xEditValue > max){
			xEditValue = min;
		}
		xEditValue = xEditValue / inc * inc;
	}
	if (event == REECCW){
		xEditValue -= inc;
		if (xEditValue < min){
			xEditValue = max;
		}
		xEditValue = xEditValue / inc * inc;
	}
}


/***
 * MQTT session is online or offline
 * @param b - true if online
 */
void DisplayAgent::online(bool b){
	xOnline = b;
}
