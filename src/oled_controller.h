/*
  usi_i2c.h

  Copyright (C) 2013 Jan Rychter
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef OLED_CONTROLLER_H
#define OLED_CONTROLLER_H

#include <stdint.h>

#include "oled_conf.h"

/// The control byte for sending commands
#define OC_CONTROL_BYTE_COMMAND 0x00

/// The control byte for sending data
#define OC_CONTROL_BYTE_DATA 0x40

/// Flag to provide to @ref oc_send to tell it to repeat the first payload byte
#define OC_REPEAT_PAYLOAD 1

/// Flag to provide to @ref oc_send to tell it to NOT repeat the first payload byte, and instead send the whole payload as provided
#define OC_DO_NOT_REPEAT_PAYLOAD 0

typedef enum oc_state_enum {
  OC_IDLE = 0,
  OC_START = 2,
  OC_PREPARE_ACKNACK = 4,
  OC_HANDLE_RXTX = 6,
  OC_PREPARE_STOP = 8,
  OC_STOP = 10
} oc_state_t;

extern oc_state_t oc_state;

/**
 * Initializes the OLED controller.
 * Call this with one of the USIDIV_* constants as a usi_clock_divider parameter, which will set the clock divider used
 * for USI I2C communications. The usi_clock_source parameter should be set to one of the USISSEL* constants.
 * Example: @code{.c} i2c_init(USIDIV_5, USISSEL_2) // uses SMCLK/16. @endcode
 */
void oc_init(uint16_t usi_clock_divider, uint16_t usi_clock_source);

/**
 * Sends a START condition, address, control byte, and data sequence.
 * Every transmission begins with a START and ends with a STOP.
 * Will busy-spin if another transmission is in progress.
 * @param control_byte is the control byte that should be passed after the I2C address. either @ref OC_CONTROL_BYTE_COMMAND or @ref OC_CONTROL_BYTE_DATA
 * @param sequence is the sequence of bytes to send after the control byte.
 * @param sequence_length is the length of @ref sequence.
 * @param wakeup_sr_bits should be a bit mask of bits to clear in the SR register when the transmission is completed
 * (to exit LPM0: LPM0_bits (CPUOFF), for LPM3: LPM3_bits (SCG1+SCG0+CPUOFF))
 * Example: @code{.c} 
 * uint8_t sequence[1] = { 0xAF };
 * oc_send_sequence(OC_CONTROL_BYTE_COMMAND, &sequence[0], 1, LPM0_bits) // uses SMCLK/16. @endcode
 */
void oc_send(uint8_t control_byte, uint8_t const * payload, uint16_t payload_length, uint8_t should_repeat_payload, uint16_t wakeup_sr_bits);

/**
 * @brief Use this to check whether a previously scheduled I2C sequence has been fully processed.
 * @return unsigned int 1 if the controller is ready, 0 otherwise.
 */
inline unsigned int oc_done() {
  return(oc_state == OC_IDLE);
}

#endif // #define OLED_CONTROLLER_H
