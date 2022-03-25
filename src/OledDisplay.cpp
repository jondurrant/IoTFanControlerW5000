/*
 * OledDisplay.cpp
 *
 *  Created on: 16 Mar 2022
 *      Author: jondurrant
 */

#include "OledDisplay.h"
#include "string.h"

OledDisplay::OledDisplay(uint8_t sda, uint8_t sdl) {
	pI2C =  i2c0;

	switch(sda){
		case 2:
		case 6:
		case 10:
		case 14:
		case 31:
			pI2C = i2c1;
			break;
		default:
			break;
	}

	i2c_init(pI2C, 400000);

	gpio_set_function(sda, GPIO_FUNC_I2C);
	gpio_set_function(sdl, GPIO_FUNC_I2C);
	gpio_pull_up(sda);
	gpio_pull_up(sdl);

	xDisp.external_vcc=false;
	ssd1306_init(&xDisp, 128, 64, 0x3C, i2c1);
	ssd1306_clear(&xDisp);
	ssd1306_show(&xDisp);
}

OledDisplay::~OledDisplay() {
	// TODO Auto-generated destructor stub
}


void OledDisplay::displayString(const char *l1, const char *l2, uint8_t scale){

	ssd1306_clear(&xDisp);

	strcpy(xBuf, l1);
	ssd1306_draw_string(&xDisp, 0, 2, scale, xBuf);

	strcpy(xBuf, l2);
	ssd1306_draw_string(&xDisp, 0, 26, scale, xBuf);

	ssd1306_show(&xDisp);
}
