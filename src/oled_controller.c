/**
 * @brief 
 * @author Stefan Antoszko
 * @file oled_controller.c

    Modified by Stefan Antoszko Nov 20 2024

    Changed to specifically work for driving a SSD2306
    I plan to remove the 16-bit bus pirate convention and swap it for a 8-bit data stream
    Remove Read support
    Always have a 2 bit header: Address, control bit.
    give the option to just repeat the first byte of the payload

    derived from usi_i2c.c by Jan Rychter

* @copyright
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

#include <msp430.h>
#include <stdint.h>
#include "oled_controller.h"

// Internal state
static uint8_t oc_control_byte;
static uint8_t const *oc_payload;
static uint16_t oc_payload_length;
static uint16_t oc_wakeup_sr_bits;
static uint8_t oc_should_repeat_payload;

typedef enum oc_internal_state_enum {
    OC_SEND_ADDRESS = 0,
    OC_SEND_CONTROL = 2,
    OC_SEND_PAYLOAD = 4
} oc_internal_state_t;

// two state machines. 
oc_state_t oc_state = OC_IDLE;
oc_internal_state_t oc_internal_state = OC_SEND_ADDRESS;

static inline void oc_prepare_stop();
static inline void oc_prepare_data_xmit();

void oc_send(uint8_t control_byte, uint8_t const * payload, uint16_t payload_length, uint8_t should_repeat_payload, uint16_t wakeup_sr_bits) {
    while(oc_state != OC_IDLE); // we can't start another payload until the current one is done
    oc_control_byte = control_byte;
    oc_payload = payload;
    oc_payload_length = payload_length;
    oc_should_repeat_payload = should_repeat_payload;
    oc_wakeup_sr_bits = wakeup_sr_bits;
    oc_state = OC_START;
    oc_internal_state = OC_SEND_ADDRESS;
    USICTL1 |= USIIFG;            // actually start communication
}

static inline void oc_prepare_stop() {
    USICTL0 |= USIOE;             // SDA = output
    USISRL = 0x00;
    USICNT |=  0x01;              // Bit counter= 1, SCL high, SDA low
    oc_state = OC_STOP;
}

static inline void oc_prepare_data_xmit() {
    if(oc_payload_length == 0) {
        oc_prepare_stop();         // nothing more to do, prepare to send STOP
    } else {
        
        USICTL0 |= USIOE;                // SDA = output

        // set output based on state
        switch(__even_in_range(oc_internal_state, OC_SEND_PAYLOAD)) {
            case OC_SEND_ADDRESS:
                USISRL = OLED_ADDR; // load address
                oc_internal_state = OC_SEND_CONTROL;
                break;
            case OC_SEND_CONTROL:
                USISRL = oc_control_byte;
                oc_internal_state = OC_SEND_PAYLOAD;
                break;
            default: // OC_SEND_PAYLOAD
                USISRL = (uint8_t)(*oc_payload);
                if(__even_in_range(oc_should_repeat_payload, OC_REPEAT_PAYLOAD) == OC_DO_NOT_REPEAT_PAYLOAD) {
                    oc_payload++;
                }
                oc_payload_length--;
                break;
        }
        // at this point we should have a pure data byte, not a command, so (*oc_sequence >> 8) == 0
        USICNT = (USICNT & 0xE0) | 8;    // Bit counter = 8, start TX
        oc_state = OC_PREPARE_ACKNACK; // next state: prepare to receive data ACK/NACK
    }
}

#ifdef __GNUC__
__attribute__((interrupt(USI_VECTOR)))
#else
#pragma vector = USI_VECTOR
__interrupt
#endif
void USI_TXRX(void)
{
    switch(__even_in_range(oc_state, OC_STOP)) {
    case OC_IDLE:
        break;

    case OC_START:               // generate start condition
        USISRL = 0x00;
        USICTL0 |= (USIGE|USIOE);
        USICTL0 &= ~USIGE;
        oc_prepare_data_xmit();
        break;

    case OC_PREPARE_ACKNACK:      // prepare to receive ACK/NACK
        USICTL0 &= ~USIOE;           // SDA = input
        USICNT |= 0x01;              // Bit counter=1, receive (N)Ack bit
        oc_state = OC_HANDLE_RXTX; // Go to next state: check ACK/NACK and continue xmitting/receiving if necessary
        break;

    case OC_HANDLE_RXTX:         // Process Address Ack/Nack & handle data TX
        if((USISRL & BIT0) != 0) {  // did we get a NACK?
            oc_prepare_stop();
        } else {
            oc_prepare_data_xmit();
        }
        break;

    case OC_PREPARE_STOP:        // prepare stop condition
        oc_prepare_stop();         // prepare stop, go to state 14 next
        break;

    case OC_STOP:                // Generate Stop Condition
        USISRL = 0xFF;             // USISRL = 1 to release SDA
        USICTL0 |= USIGE;           // Transparent latch enabled
        USICTL0 &= ~(USIGE|USIOE);  // Latch/SDA output disabled
        oc_state = OC_IDLE;       // Reset state machine for next xmt
        if(oc_wakeup_sr_bits) {
            _bic_SR_register_on_exit(oc_wakeup_sr_bits); // exit active if prompted to
        }
        break;
    }
    USICTL1 &= ~USIIFG;           // Clear pending flag
}

void oc_init(uint16_t usi_clock_divider, uint16_t usi_clock_source) {
    _disable_interrupts();
    USICTL0 = USIPE6|USIPE7|USIMST|USISWRST;  // Port & USI mode setup
    USICTL1 = USII2C|USIIE;                   // Enable I2C mode & USI interrupt
    USICKCTL = usi_clock_divider | usi_clock_source | USICKPL;
    USICNT |= USIIFGCC;                       // Disable automatic clear control
    USICTL0 &= ~USISWRST;                     // Enable USI
    USICTL1 &= ~USIIFG;                       // Clear pending flag
    _enable_interrupts();
}
