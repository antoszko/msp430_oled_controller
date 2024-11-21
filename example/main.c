/**
 * HEY stupid idiot!!!!! Make sure to physically look at the device in the
 * programmer! Is it a MSP430G2211?? That device doesn't have a USI peripheral!
 *
 * - Stefan Oct 13, 2024 aghhrhghghhh
 */

#include <msp430.h> 

//#include "usi_i2c.h"
#include "oled_controller.h"

#include "oled.h"

const uint8_t OLED_mockup_header[2*34] = {
0x80, 0xc0, 0x00, 0x70, 0x98, 0x38, 0x78, 0x30, 0x00, 0x78, 0xc0, 0x98, 0xf8, 0xf8, 0x00, 0xf8, 0xf8, 0xf0, 0x00, 0xf0, 0x00, 0x78, 0xf8, 0x08, 0xf8, 0x18, 0x70, 0x00, 0xf8, 0x98, 0xc0, 0xf0, 0x78, 0x38,
0x03, 0x07, 0x07, 0x04, 0x05, 0x07, 0x00, 0x03, 0x07, 0x04, 0x06, 0x06, 0x07, 0x03, 0x00, 0xe7, 0x80, 0x63, 0x06, 0xe7, 0x00, 0x00, 0x04, 0x06, 0x07, 0x00, 0x06, 0x01, 0x04, 0x07, 0x07, 0x01, 0x00, 0x00
};

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	BCSCTL1 = 0x7F;                    // Set DCO
	DCOCTL = 0x87;

	oc_init(USIDIV_7, USISSEL_2);

    oled_init();
    oled_fill_pages(0, 4, 0x00);
    oled_draw_checker();
    oled_fill_pages(0, 2, 0x00);

    uint8_t i = 0;
    // this decal is 2 pages tall
    for(; i < 2; i++) {
        // set y (page) offset = 0
        oled_send_cmd(0xB0 + 0 + i);
        // set x offset = 46
        oled_send_cmd(0x00 + (46 & 0x0f));
        oled_send_cmd(0x10 + ((46 & 0xf0) >> 4));

        oled_send_data(&OLED_mockup_header[34*i], 34);
    }

    // draw lines
    // set y (page) offset = 0
    oled_send_cmd(0xB0 + 0);
    // set x offset = 5
    oled_send_cmd(0x00 + (5 & 0x0f));
    oled_send_cmd(0x10 + ((5 & 0xf0) >> 4));
    oled_send_data_repeated(0x80, 38);

    // set x offset = 82
    oled_send_cmd(0x00 + (82 & 0x0f));
    oled_send_cmd(0x10 + ((82 & 0xf0) >> 4));
    oled_send_data_repeated(0x80, 41);

    while(1) {
        LPM3;
    }
}
