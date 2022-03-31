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
#include <MQTTConfig.h>
#include <math.h>



/***
 * State will have 4 element two from StateTemperature and two added here
 */
FanState::FanState() {
	elements=14;

	jsonHelpers[FAN_ON_SLOT] = (StateFunc)&FanState::jsonOn;
	jsonHelpers[FAN_DAY_END_SLOT] = (StateFunc)&FanState::jsonDayEnd;
	jsonHelpers[FAN_DAY_START_SLOT] = (StateFunc)&FanState::jsonDayStart;
	jsonHelpers[FAN_DAY_SLOT] = (StateFunc)&FanState::jsonDay;
	jsonHelpers[FAN_CLOCK_SLOT] = (StateFunc)&FanState::jsonClock;
	jsonHelpers[FAN_CSPEED_SLOT] = (StateFunc)&FanState::jsonCSpeed;
	jsonHelpers[FAN_MAX_NIGHT_SLOT] = (StateFunc)&FanState::jsonMSpeed;
	jsonHelpers[FAN_PRE_TEMP_SLOT] = (StateFunc)&FanState::jsonPreTemp;
	jsonHelpers[FAN_PRE_SPEED_SLOT] = (StateFunc)&FanState::jsonPreSpeed;
	jsonHelpers[FAN_ENV_TEMP_SLOT] = (StateFunc)&FanState::jsonEnvTemp;
	jsonHelpers[FAN_OVERRIDE_SLOT] = (StateFunc)&FanState::jsonOverrideMinutes;


	memset(xPreTemp,  0, FAN_PRESETS);
	memset(xPreSpeed, 0, FAN_PRESETS);

	xDS18B20.DS18Initalize(pio0, 13);
	xDS18B20.convert();
}

/***
 * Destructor
 */
FanState::~FanState() {
	// TODO Auto-generated destructor stub
}

/***
 * calculate Speed
 * @return
 */
void FanState::calcSpeed(){

	//float t = getTemp();
	float t = getEnvTemp();
	uint8_t speed = 0;

	if (getOverrideMinutes()>0){
		LogDebug(("Fan Speed is overriden"));
		return;
	}

	if (t < getPreTemp()[0]){
		setCurrentSpeed(speed);
		LogDebug(("Setting %d speed for %fC",speed, t));
		return;
	}

	speed = 100;
	for (int8_t i = FAN_PRESETS - 1; i >= 0; i--){
		if (t >= (float)getPreTemp()[i]){
			speed = getPreSpeed()[i];
			setCurrentSpeed(speed);
			LogDebug(("Setting %d speed for %fC",speed, t));

			return;
		}
	}


	LogDebug(("Setting %d speed for %fC",speed, t));
	setCurrentSpeed(speed);
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
	if (!getOn()){
		return 0;
	}
	if (! isDay()){
		if (xCSpeed > getMaxNightSpeed()){
			return getMaxNightSpeed();
		}
	}
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
 * Get Max Night Speed
 * @return %0-100
 */
uint8_t FanState::getMaxNightSpeed(){
	return xMSpeed;
}


/***
 * Set Max Night Speed
 * @param percent - 0 to 100
 */
void FanState::setMaxNightSpeed(uint8_t percent){
	if (percent <= 100){
		xMSpeed = percent;
		setDirty(FAN_MAX_NIGHT_SLOT);
	}
}


const uint8_t * FanState::getPreTemp() const{
	return xPreTemp;
}

void FanState::setPreTemp(uint8_t* temps){
	memcpy(xPreTemp, temps, FAN_PRESETS);
	setDirty(FAN_PRE_TEMP_SLOT);
}

void FanState::setPreTemp(uint8_t temp, uint8_t preset){
	if (preset < FAN_PRESETS){
		xPreTemp[preset] = temp;
		setDirty(FAN_PRE_TEMP_SLOT);
	}
}

const uint8_t * FanState::getPreSpeed() const{
	return xPreSpeed;
}

void FanState::setPreSpeed(uint8_t* speeds){
	memcpy(xPreSpeed, speeds, FAN_PRESETS);
	setDirty(FAN_PRE_SPEED_SLOT);
}

void FanState::setPreSpeed(uint8_t speed, uint8_t preset){
	if (preset < FAN_PRESETS){
		xPreSpeed[preset] = speed;
		setDirty(FAN_PRE_SPEED_SLOT);
	}
}


/***
 * Set temp in celsius
 * @param temp
 */
void FanState::setEnvTemp(float temp){
	if (temp != xEnvTemp){
		xEnvTemp = temp;
		setDirty(FAN_ENV_TEMP_SLOT);
	}
}


/***
 * Get Environment Temperature
 * @return celsius
 */
float FanState::getEnvTemp(){
	return xEnvTemp;
}


/***
 * Number of minutes left for fan speed override
 * @return
 */
uint16_t FanState::getOverrideMinutes(){
	return xOverrideMinutes;
}

/***
 * Set number of minutes to override fan speed for
 * @param min
 */
void FanState::setOverrideMinutes(uint16_t min){
	xOverrideMinutes = min;
	setDirty(FAN_OVERRIDE_SLOT);
}





/***
* Update time and temp and trigger state update
*/
void FanState::updateClock(){
	datetime_t time;
	rtc_get_datetime(&time);
	if ( (time.hour > getDayStart()) && (time.hour < getDayEnd()) ){
		if (!isDay()){
			setDay(true);
		}
	} else {
		if (isDay()){
			setDay(false);
		}
	}

	if (time.min != xMin){
		xMin = time.min;
		if (getOverrideMinutes() > 0){
			setOverrideMinutes(getOverrideMinutes() -1);
		}
	}

	setDirty(FAN_CLOCK_SLOT);
	updateTemp();

	setEnvTemp(xDS18B20.getTemperature());
	calcSpeed();

	xDS18B20.convert();
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
* Retried Max Night Speed in JSON format
* @param buf
* @param len
* @return
*/
char* FanState::jsonMSpeed(char *buf, unsigned int len){
	char *p = buf;
	p = json_uint( p, "maxSpeed", getMaxNightSpeed(), &len);
	return p;
}


/***
 * Retried Preset Temp in JSON format
 * @param buf
 * @param len
 * @return
 */
char* FanState::jsonPreTemp(char *buf, unsigned int len){
	char *p = buf;
	p = json_arrOpen( p, "preTemp", &len);
	for (unsigned char i=0; i < FAN_PRESETS; i++){
		p = json_uint( p, NULL, getPreTemp()[i], &len );
	}
	p = json_arrClose( p, &len);
	return p;
}

/***
 * Retried Preset Speed in JSON format
 * @param buf
 * @param len
 * @return
 */
char* FanState::jsonPreSpeed(char *buf, unsigned int len){
	char *p = buf;
	p = json_arrOpen( p, "preSpeed", &len);
	for (unsigned char i=0; i < FAN_PRESETS; i++){
		p = json_uint( p, NULL, getPreSpeed()[i], &len );
	}
	p = json_arrClose( p, &len);
	return p;
}

/***
* Retried Preset Speed in JSON format
* @param buf
* @param len
* @return
*/
char* FanState::jsonEnvTemp(char *buf, unsigned int len){
	char *p = buf;
	p = json_double( p, "envTemp", getEnvTemp(), &len);
	return p;
}

/***
 * Fan speed override time in minutes JSON format
 * @param buf
 * @param len
 * @return
 */
char* FanState::jsonOverrideMinutes(char *buf, unsigned int len){
	char *p = buf;
	p = json_uint( p, "override", getOverrideMinutes(), &len);
	return p;
}



/***
* Update state data from a json structure
* @param json
*/
void FanState::updateFromJson(json_t const *json){
	StateTemp::updateFromJson(json);

	json_t const *jp;
	uint8_t presets[FAN_PRESETS];
	bool presetsOk = true;
	memset(presets, 0, FAN_PRESETS);

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

	jp = json_getProperty(json, "maxSpeed");
	if (jp){
		if (JSON_INTEGER == json_getType(jp)){
			setMaxNightSpeed(json_getInteger(jp));
		}
	}

	jp = json_getProperty(json, "preTemp");
	if (jp){
		if (JSON_ARRAY == json_getType(jp)){
			jp = json_getChild(jp);
			for (unsigned char i=0; i < FAN_PRESETS; i++){
				if (jp){
					if (JSON_INTEGER == json_getType(jp)){
						int j = json_getInteger(jp);
						if ((j >= 0) && (j <= 0xFF)){
							presets[i] = (uint8_t) j;
						} else {
							presetsOk = false;
						}
					}
					else {
						presetsOk = false;
					}
				} else {
					presetsOk = false;
				}
				jp = json_getSibling(jp);
			}
			if (presetsOk){
				setPreTemp(presets);
			}
		}
	}

	jp = json_getProperty(json, "preSpeed");
	if (jp){
		if (JSON_ARRAY == json_getType(jp)){
			jp = json_getChild(jp);
			for (unsigned char i=0; i < FAN_PRESETS; i++){
				if (jp){
					if (JSON_INTEGER == json_getType(jp)){
						int j = json_getInteger(jp);
						if ((j >= 0) && (j <= 0xFF)){
							presets[i] = (uint8_t) j;
						} else {
							presetsOk = false;
						}
					}
					else {
						presetsOk = false;
					}
				} else {
					presetsOk = false;
				}
				jp = json_getSibling(jp);
			}
			if (presetsOk){
				setPreSpeed(presets);
			}
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



