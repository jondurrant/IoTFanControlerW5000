/*
 * RotEncListener.cpp
 *
 *  Created on: 24 Mar 2022
 *      Author: jondurrant
 */

#include <stdio.h>
#include "RotEncListener.h"

RotEncListener::RotEncListener() {
	// NOP

}

RotEncListener::~RotEncListener() {
	// NOP
}

void RotEncListener::shortPress(void * rotEnv){
	printf("Short press\n");
}

void RotEncListener::longPress(void * rotEnv){
	printf("Lond press\n");
}

void RotEncListener::rotate(bool clockwise, int16_t pos, void * rotEnc){
	if (clockwise){
		printf("Clockwise %d\n", pos);
	} else {
		printf("Withershins %d\n", pos);
	}
}
