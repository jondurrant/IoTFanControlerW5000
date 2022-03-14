/*
 * Fan State.cpp
 *
 * Fan State
 *
 *  Created on: 14-Mar 2022
 *      Author: jondurrant
 */

#include "FanState.h"
#include "hardware/adc.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "json-maker/json-maker.h"

#include "hardware/rtc.h"



/***
 * State will have 4 element two from StateTemperature and two added here
 */
FanState::FanState() {
	elements=9;

	jsonHelpers[FAN_ON_SLOT] = (StateFunc)&FanState::jsonOn;
	jsonHelpers[FAN_DAY_END_SLOT] = (StateFunc)&FanState::jsonDayEnd;
	jsonHelpers[FAN_DAY_START_SLOT] = (StateFunc)&FanState::jsonDayStart;
	jsonHelpers[FAN_DAY_SLOT] = (StateFunc)&FanState::jsonDay;
	jsonHelpers[FAN_CLOCK_SLOT] = (StateFunc)&FanState::jsonClock;
	jsonHelpers[FAN_CSPEED_SLOT] = (StateFunc)&FanState::jsonCSpeed;

}

/***
 * Destructor
 */
FanState::~FanState() {
	// TODO Auto-generated destructor stub
}



/***
 * Set On status of switch
 * @param bool
 */
void FanState::setOn(bool b){
	xOn = b;
	setDirty(FAN_ON_SLOT);
}

/***
 * Get on status of switch
 * @return
 */
bool FanState::getOn() const{
	return xOn;
}

/***
 * Returns hour at which Day Ends
 * @return
 */
uint8_t FanState::getDayEnd() const {
	return xDayEnd;
}

/***
 * Sets hour at which day ends
 * @param DayEnd
 */
void FanState::setDayEnd(uint8_t xDayEnd) {
	this->xDayEnd = xDayEnd;
	setDirty(FAN_DAY_END_SLOT);
}

/***
 * Get Start Day Hour
 * @return start hour
 */
uint8_t FanState::getDayStart() const {
	return xDayStart;
}

/***
 * Sets Day start hour
 * @param xDayStart
 */
void FanState::setDayStart(uint8_t xDayStart) {
	this->xDayStart = xDayStart;
	setDirty(FAN_DAY_START_SLOT);
}


/***
 * Is in day mode
 * @return
 */
bool FanState::isDay() const {
	return xDay;
}

/***
 * Set day mode
 * @param xDay
 */
void FanState::setDay(bool xDay) {
	this->xDay = xDay;

	setDirty(FAN_DAY_SLOT);
}


/***
 * Get Current Speed
 * @return %0-100
 */
uint8_t FanState::getCurrentSpeed(){
	return xCSpeed;
}


/***
 * Set Current Speed
 * @param percent - o to 100
 */
void FanState::setCurrentSpeed(uint8_t percent){
	if (percent <= 100){
		xCSpeed = percent;
		setDirty(FAN_CSPEED_SLOT);
	}
}



/***
* Update time and temp and trigger state update
*/
void FanState::updateClock(){
	setDirty(FAN_CLOCK_SLOT);
	updateTemp();
}



/***
 * Retried Seq number in JSON format
 * @param buf
 * @param len
 * @return
 */
char* FanState::jsonDay(char *buf, unsigned int len){
	char *p = buf;
	p = json_bool( p, "day", isDay(), &len);
	return p;
}





/***
 * Retried Day Start Hour status in JSON format
 * @param buf
 * @param len
 * @return
 */
char* FanState::jsonDayStart(char *buf, unsigned int len){
	char *p = buf;
	p = json_uint( p, "days", getDayStart(), &len);
	return p;
}

/***
 * Retried Day End Hour status in JSON format
 * @param buf
 * @param len
 * @return
 */
char* FanState::jsonDayEnd(char *buf, unsigned int len){
	char *p = buf;
	p = json_uint( p, "daye", getDayEnd(), &len);
	return p;
}

/***
 * Retrieve on status in JSON format
 * @param buf
 * @param len
 * @return
 */
char* FanState::jsonOn(char *buf, unsigned int len){
	char *p = buf;
    p = json_bool( p, "on", getOn(), &len);
    return p;
}


/***
 * Retrieve Time in JSON format
 * @param buf
 * @param len
 * @return
 */
char* FanState::jsonClock(char *buf, unsigned int len){
	char *p = buf;
	char s[20];
	datetime_t time;
	rtc_get_datetime(&time);
	if (time.year > 3000 || time.year < 2000)
		return buf;
	sprintf(s,"%d-%d-%d %d:%d:%d",
			time.year,  //4
			time.month, //2
			time.day,   //2
			time.hour,  //2
			time.min,   //2
			time.sec    //2  //5
			);
	p = json_str(p, "clock", s, &len);
	return p;
}

/***
* Retried Current Speed in JSON format
* @param buf
* @param len
* @return
*/
char* FanState::jsonCSpeed(char *buf, unsigned int len){
	char *p = buf;
	p = json_uint( p, "cSpeed", getCurrentSpeed(), &len);
	return p;
}



/***
* Update state data from a json structure
* @param json
*/
void FanState::updateFromJson(json_t const *json){
	StateTemp::updateFromJson(json);

	json_t const *jp;

	jp = json_getProperty(json, "on");
	if (jp){
		if (JSON_BOOLEAN == json_getType(jp)){
			bool b = json_getBoolean(jp);
			setOn(b);
		}
	}

	jp = json_getProperty(json, "days");
	if (jp){
		if (JSON_INTEGER == json_getType(jp)){
			setDayStart(json_getInteger(jp));
		}
	}

	jp = json_getProperty(json, "daye");
	if (jp){
		if (JSON_INTEGER == json_getType(jp)){
			setDayEnd(json_getInteger(jp));
		}
	}

	jp = json_getProperty(json, "day");
	if (jp){
		if (JSON_BOOLEAN == json_getType(jp)){
			setDay(json_getBoolean(jp));
		}
	}

	jp = json_getProperty(json, "cSpeed");
	if (jp){
		if (JSON_INTEGER == json_getType(jp)){
			setCurrentSpeed(json_getInteger(jp));
		}
	}

}

/***
 * Retrieve state of object in JSON format
 * @param buf - buffer to write to
 * @param len - length of buffer
 * @return length of json or zero if we ran out of space
 */
unsigned int FanState::state(char *buf, unsigned int len){
	updateTemp();
	return StateTemp::state(buf, len);
}
