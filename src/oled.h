/*
 * oled.h
 *
 *  Created on: Nov 15, 2024
 *      Author: stefan
 */

#ifndef OLED_H_
#define OLED_H_

#include <stdint.h> // for uint8_t

void oled_send_cmd(uint8_t payload);
void oled_send_cmd2(uint8_t payload0, uint8_t payload1);
void oled_send_data(const uint8_t* payload, uint16_t payload_length);

/**
 * @param num_repeats the number of times to reapeat the data byte. Total times it is transmitted is num_repeats+1.
 */
void oled_send_data_repeated(uint8_t payload, uint16_t payload_length);

void oled_init(void);
void oled_draw_checker(void);
/**
 * fills pages from start_page to end_page (exclusive) with data.
 * start_page, end_page must be <= OLED_HEIGHT / 8;
 */
void oled_fill_pages(uint8_t start_page, uint8_t end_page, uint8_t data);

#endif /* OLED_H_ */
