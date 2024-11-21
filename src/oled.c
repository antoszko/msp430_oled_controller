#include "oled_conf.h"
#include "oled.h"

#include <msp430.h>
#include "oled_controller.h"

void oled_send_cmd(uint8_t payload) {
    oc_send(OC_CONTROL_BYTE_COMMAND, &payload, 1, OC_DO_NOT_REPEAT_PAYLOAD, LPM0_bits);
    LPM0;
    while(!oc_done());
}

void oled_send_cmd2(uint8_t payload0, uint8_t payload1) {
    uint8_t payload[2] = {payload0, payload1};
    oc_send(OC_CONTROL_BYTE_COMMAND, payload, 2, OC_DO_NOT_REPEAT_PAYLOAD, LPM0_bits);
    LPM0;
    while(!oc_done());
}

void oled_send_data(const uint8_t* payload, uint16_t payload_length) {

    oc_send(OC_CONTROL_BYTE_DATA, payload, payload_length, OC_DO_NOT_REPEAT_PAYLOAD, LPM0_bits);
    LPM0;
    while(!oc_done());
}

void oled_send_data_repeated(uint8_t payload, uint16_t payload_length) {
    oc_send(OC_CONTROL_BYTE_DATA, &payload, payload_length, OC_REPEAT_PAYLOAD, LPM0_bits);
    LPM0;
    while(!oc_done());
}

// -------------- helpers ------------------

void oled_init(void) {

    oled_send_cmd(0x20); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
    oled_send_cmd(0x00); // 10b,Page Addressing Mode (RESET); 11b,Invalid


    oled_send_cmd(0xB0); // Set Page Start Address for Page Addressing Mode,0-7

    oled_send_cmd(0xC8); // Set COM Output Scan Direction (remap to scan from COM[N-1] to COM0.

//    oled_write_cmd(0x00); //---set low column address to be ( 0 )
//    oled_write_cmd(0x10); //---set high column address      ( to be 0 )

    oled_send_cmd(0x40); //--set start line address - CHECK (set it to be 0.)

#if OLED_BRIGHTNESS != 0x7f
    oled_send_cmd2(0x81, OLED_BRIGHTNESS); // Set contrast
#endif

    oled_send_cmd(0xA1); //--set segment re-map 0 to 127 - CHECK (column address 127 is mapped to SEG0)

    oled_send_cmd(0xA6); //--set normal color (not inverted) (default)

    oled_send_cmd(0xA8); //--set multiplex ratio(1 to 64) - CHECK
#if OLED_HEIGHT == 32
    oled_send_cmd(0x1F); // if 32 pixel screen ( set to be 16MUX)
#elif OLED_HEIGHT == 64
    oled_write_cmd(0x3F); // if 64 pixel screen
#else
#error "OLED : invalid OLED_HEIGHT! Allowed values are 32, 64."
#endif
    oled_send_cmd(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

//    oled_write_cmd(0xD3); //-set display offset - CHECK (default)
//    oled_write_cmd(0x00); //-not offset

    oled_send_cmd(0xD5); //--set display clock divide ratio/oscillator frequency
    oled_send_cmd(0xF0); //--set divide ratio (set to max)

//    oled_write_cmd(0xD9); //--set pre-charge period (22 is default)
//    oled_write_cmd(0x22); //

    oled_send_cmd(0xDA); //--set com pins hardware configuration - CHECK
#if OLED_HEIGHT == 32
    oled_send_cmd(0x02); // if 32 pixel screen (sequential COM pin configuration, Disable COM Left/Right remap)
#elif OLED_HEIGHT == 64
    oled_write_cmd(0x12);  // if 64 pixel screen
#else
#error "OLED : invalid OLED_HEIGHT! Allowed values are 32, 64."
#endif

//    oled_write_cmd(0xDB); //--set vcomh (default is 0x20)
//    oled_write_cmd(0x20); //0x20,0.77xVcc

    oled_send_cmd(0x8D); // Enable charge pump during display on
    oled_send_cmd(0x14); //

//    oled_write_cmd(0xA5); // turn whole screen on
    oled_send_cmd(0xAF);
}

void oled_fill_pages(uint8_t start_page, uint8_t end_page, uint8_t data) {

    __even_in_range(start_page, OLED_HEIGHT/8);
    __even_in_range(end_page, OLED_HEIGHT/8);

    oled_send_cmd(0xB0);
    oled_send_cmd(0x00);
    oled_send_cmd(0x10);

    uint8_t i = start_page;
    for(; i < end_page; i++) {

        oled_send_data_repeated(data, 128);
    }
}

void oled_draw_checker(void) {
    oled_send_cmd(0xB0);
    oled_send_cmd(0x00);
    oled_send_cmd(0x10);

    uint8_t i = 0;
    for(; i < OLED_HEIGHT/8; i++) {


        uint8_t j = 0;
        for(; j < 128/8; j++) {
            oled_send_data_repeated(0xf0, 4);
            oled_send_data_repeated(0x0f, 4);
        }
    }
}
