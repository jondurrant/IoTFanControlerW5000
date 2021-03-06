/*
 * FanState.h
 *
 * Fan State
 *
 *  Created on: 14-Mar- 2022
 *      Author: jondurrant
 */

#ifndef FANSTATE_H_
#define FANSTATE_H_

#include <MQTTConfig.h>
#include "StateTemp.h"
#include <stdbool.h>
#include <stdint.h>
#include <DS18B20.h>

#define FAN_ON_SLOT 		3
#define FAN_DAY_END_SLOT 	4
#define FAN_DAY_START_SLOT	5
#define FAN_DAY_SLOT 		6
#define FAN_CLOCK_SLOT		7
#define FAN_CSPEED_SLOT		8
#define FAN_MAX_NIGHT_SLOT	9
#define FAN_PRE_TEMP_SLOT	10
#define FAN_PRE_SPEED_SLOT	11
#define FAN_ENV_TEMP_SLOT	12
#define FAN_OVERRIDE_SLOT	13


#define FAN_PRESETS 3

/***
 * State include temperate, Fan Speed
 */
class FanState: public StateTemp {
public:
	FanState();
	virtual ~FanState();

	/***
	 * calculate Speed
	 * @return
	 */
	void calcSpeed();

	/***
	 * Gets the On status of the fan
	 * @return
	 */
	bool getOn() const;

	/***
	 * Sets the on status of the fan
	 * @param b
	 */
	void setOn(bool b);

	/***
	 * Returns hour at which Day Ends
	 * @return
	 */
	uint8_t getDayEnd() const ;

	/***
	 * Sets hour at which day ends
	 * @param DayEnd
	 */
	void setDayEnd(uint8_t xDayEnd = 21) ;

	/***
	 * Get Start Day Hour
	 * @return start hour
	 */
	uint8_t getDayStart() const ;

	/***
	 * Sets Day start hour
	 * @param xDayStart
	 */
	void setDayStart(uint8_t xDayStart = 7) ;

	/***
	 * Is in day mode
	 * @return
	 */
	bool isDay() const ;

	/***
	 * Set day mode
	 * @param xDay
	 */
	void setDay(bool xDay = true) ;


	/***
	 * Get Environment Temperature (DS18B20)
	 * @return celsius
	 */
	float getEnvTemp();


	/***
	 * Set temp in celsius
	 * @param temp
	 */
	void setEnvTemp(float temp);

	/***
	 * Number of minutes left for fan speed override
	 * @return
	 */
	uint16_t getOverrideMinutes();

	/***
	 * Set number of minutes to override fan speed for
	 * @param min
	 */
	void setOverrideMinutes(uint16_t min);


	/***
	 * Update time and temp and trigger state update
	 */
	void updateClock();


	/***
	 * Get Current speed
	 * @return percentage 0 to 100
	 */
	uint8_t getCurrentSpeed();

	/***
	 * Set Current Speed
	 * @param percent: 0 to 100
	 */
	void setCurrentSpeed(uint8_t percent);

	/***
	 * Get Maximum speed to run at night
	 * @return perecntage 0 to 100
	 */
	uint8_t getMaxNightSpeed();

	/***
	 * Set Maximum speed to run at night
	 * @param percent - 0 to 100
	 */
	void setMaxNightSpeed(uint8_t percent);

	/***
	 * Get Temperate presets
	 * When temp < t[0] speed is zero
	 * When t[i] <= temp < t[i+1]: speed is speed[i[
	 * When temp > t[2]: speed is speed[2]
	 * @return uint8_t[3]
	 */
	const uint8_t * getPreTemp() const;

	/***
	 * Set Temperature presets
	 * @param temps - uint8_t[3]
	 */
	void setPreTemp(uint8_t* temps);

	/***
	 * Set specific temp preset
	 * @param temp - tempereature
	 * @param preset - 0 to 2.
	 */
	void setPreTemp(uint8_t temp, uint8_t preset);

	/***
	 * Get preset speed
	 * @return uintt_t[3], each value 0 to 100
	 */
	const uint8_t * getPreSpeed() const;

	/***
	 * Set preset speeds
	 * When temp < t[0] speed is zero
	 * When t[i] <= temp < t[i+1]: speed is speed[i[
	 * When temp > t[2]: speed is speed[2]
	 * @param speeds uint8_t[3] - each value 0 to 100
	 */
	void setPreSpeed(uint8_t* speeds);

	/***
	 * Set specific preset speed
	 * @param speed - 0 to 100
	 * @param preset 0 to 2
	 */
	void setPreSpeed(uint8_t speed, uint8_t preset);



	/***
	 * Update state data from a json structure
	 * @param json
	 */
	virtual void updateFromJson(json_t const *json);

	/***
	 * Retrieve state of object in JSON format
	 * @param buf - buffer to write to
	 * @param len - length of buffer
	 * @return length of json or zero if we ran out of space
	 */
	virtual unsigned int state(char *buf, unsigned int len) ;


protected:
	/***
	 *  On status in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonOn(char *buf, unsigned int len);

	/***
	 *  Day Start Hour status in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonDayStart(char *buf, unsigned int len);

	/***
	 *  Day End Hour status in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonDayEnd(char *buf, unsigned int len);

	/***
	 *  Day in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonDay(char *buf, unsigned int len);

	/***
	 *  Time in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonClock(char *buf, unsigned int len);

	/***
	 *  Current Speed in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonCSpeed(char *buf, unsigned int len);

	/***
	 *  Max Night Speed in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonMSpeed(char *buf, unsigned int len);

	/***
	 *  Preset Temp in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonPreTemp(char *buf, unsigned int len);

	/***
	 *  Preset Speed in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonPreSpeed(char *buf, unsigned int len);

	/***
	 *  Preset Speed in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonEnvTemp(char *buf, unsigned int len);

	/***
	 * Fan speed override time in minutes JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonOverrideMinutes(char *buf, unsigned int len);

private:
	//Is Fan on
	bool xOn = false;

	//Day start and end
	uint8_t xDayStart = 7;
	uint8_t xDayEnd = 21;
	bool xDay = true;

	//Fan Speed
	uint8_t xCSpeed = 100;
	uint8_t xMSpeed = 100;

	//Presets
	uint8_t xPreTemp[FAN_PRESETS];
	uint8_t xPreSpeed[FAN_PRESETS];

	//Environment temp read by DS18B20
	float xEnvTemp = 0.0;

	//Hardware interface
	DS18B20 xDS18B20 ;

	uint8_t xMin = 0;
	uint16_t xOverrideMinutes = 0;

};

#endif /* STATETEMP_H_ */
