/*
 * OledDisplay.h
 *
 *  Created on: 16 Mar 2022
 *      Author: jondurrant
 */

#ifndef OLEDDISPLAY_H_
#define OLEDDISPLAY_H_

#include <stdint.h>
extern "C" {
#include "ssd1306.h"
}

class OledDisplay {
public:
	OledDisplay(uint8_t sda, uint8_t sdl);
	virtual ~OledDisplay();

	void displayString(const char *s, uint8_t scale=1);

protected:
	ssd1306_t xDisp;
	i2c_inst_t *pI2C =  i2c0;
	char xBuf[80];

};

#endif /* OLEDDISPLAY_H_ */
