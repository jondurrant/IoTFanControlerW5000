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

#define FAN_ON_SLOT 		3
#define FAN_DAY_END_SLOT 	4
#define FAN_DAY_START_SLOT	5
#define FAN_DAY_SLOT 		6
#define FAN_CLOCK_SLOT		7
#define FAN_CSPEED_SLOT		8


/***
 * State include temperate, Fan Speed
 */
class FanState: public StateTemp {
public:
	FanState();
	virtual ~FanState();

	/***
	 * Gets the On status of the light
	 * @return
	 */
	bool getOn() const;

	/***
	 * Sets the on status of the light
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
	 * Update time and temp and trigger state update
	 */
	void updateClock();


	uint8_t getCurrentSpeed();

	void setCurrentSpeed(uint8_t percent);


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
	 * Retried On status in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonOn(char *buf, unsigned int len);

	/***
	 * Retried Day Start Hour status in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonDayStart(char *buf, unsigned int len);

	/***
	 * Retried Day End Hour status in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonDayEnd(char *buf, unsigned int len);

	/***
	 * Retried Day in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonDay(char *buf, unsigned int len);

	/***
	 * Retried Time in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonClock(char *buf, unsigned int len);

	/***
	 * Retried Current Speed in JSON format
	 * @param buf
	 * @param len
	 * @return
	 */
	char* jsonCSpeed(char *buf, unsigned int len);

private:
	//Is Fan on
	bool xOn = false;

	//Day start and end
	uint8_t xDayStart = 7;
	uint8_t xDayEnd = 21;
	bool xDay = true;

	//Fan Speed
	uint8_t xCSpeed = 0;

};

#endif /* STATETEMP_H_ */
