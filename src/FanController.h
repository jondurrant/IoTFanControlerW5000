/*
 * FanController.h
 *
 * PWM Fan Controller
 *
 *  Created on: 15 Mar 2022
 *      Author: jondurrant
 */

#ifndef FANCONTROLLER_H_
#define FANCONTROLLER_H_

#include <StateObserver.h>
#include "FanState.h"
#include "stdint.h"

class FanController : public StateObserver{
public:
	/***
	 * Constructor
	 * @param state - State object used to find speed
	 * @param gp - GPIO number for PWM pin
	 */
	FanController(FanState *state, uint8_t gp=0);

	/***
	 * Destructor
	 */
	virtual ~FanController();

	/***
	 * Notification of a change of a state item with the State object.
	 * @param dirtyCode - Representation of item changed within state. Used to pull back delta
	 */
	virtual void notifyState(uint16_t dirtyCode);

private:
	uint8_t    xPin   = 0;
	FanState * pState = NULL;


};

#endif /* FANCONTROLLER_H_ */
