/*
 * oled_conf.h
 *
 *  Created on: Nov 15, 2024
 *      Author: stefan
 */

#ifndef OLED_CONF_H_
#define OLED_CONF_H_

#define OLED_ADDR (0x3C << 1)

// from 0-255.
#define OLED_BRIGHTNESS 32
#define OLED_HEIGHT 32

#if OLED_HEIGHT != 32 && OLED_HEIGHT != 64
#error "OLED : invalid OLED_HEIGHT! Allowed values are 32, 64."
#endif

#endif /* OLED_CONF_H_ */
