/*
 * FanController.cpp
 *
 *  Created on: 15 Mar 2022
 *      Author: jondurrant
 */

#include "FanController.h"
#include "hardware/pwm.h"

FanController::FanController(FanState *state, uint8_t gp) {
	xPin = gp;
	pState = state;

	pState->attach(this);

	 gpio_set_function(xPin, GPIO_FUNC_PWM);
	 uint slice_num = pwm_gpio_to_slice_num(0);
	 pwm_set_enabled(slice_num, true);
	 pwm_set_gpio_level	(xPin,0);
}

FanController::~FanController() {
	// TODO Auto-generated destructor stub
}


/***
 * Notification of a change of a state item with the State object.
 * @param dirtyCode - Representation of item changed within state. Used to pull back delta
 */
void FanController::notifyState(uint16_t dirtyCode){
	if ((dirtyCode | FAN_CSPEED_SLOT) > 0){
		uint8_t speed = pState->getCurrentSpeed();

		uint16_t s = 0xffff/100;
		uint16_t minSpeed = s * 20;
		s = (0xffff - minSpeed)/100;
		s = (speed * s) + minSpeed;

		if (speed == 0){
			s = 0;
		}

		if (!pState->getOn()){
			s = 0;
		}
		pwm_set_gpio_level	(xPin, s);
	}
}
