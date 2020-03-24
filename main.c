#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/delay.h>

#include "Util/communication.h"
#include "Util/controller.h"
#include "Util/output.h"
#include "HAL/timer8bit.h"
#include "Util/input.h"
#include "Drivers/bno055_uart.h"
#include "HAL/uart.h"

volatile state_t curr_state;
volatile setpoint_t curr_setpoint;
volatile out_state_t out_state;
volatile uint8_t usbTimeout = 0, sbusTimeout = 0;

#define NORMALIZE_TARANIS(x) ((uint16_t)(x-172)*(1000.0F/(1811-172)))

typedef enum {
    failsave, remote, flightcomputer
} setpoint_source_t;

volatile setpoint_source_t setpoint_source = failsave;

void setpoint_update(setpoint_t setpoint) {
    if (setpoint_source == flightcomputer) {
        curr_setpoint.roll = setpoint.roll;
        curr_setpoint.pitch = setpoint.pitch;
        curr_setpoint.power = setpoint.power;
    }
    output_led(3, toggle);
    usbTimeout = 0;
}

void sbus_event(sbus_data_t sbus_data) {
    if (sbus_data.failsave) {
        setpoint_source = failsave;
    } else if (sbus_data.channel[7] < 500) {
        setpoint_source = remote;
    } else {
        setpoint_source = flightcomputer;
    }
    output_led(5, toggle);
    sbusTimeout = 0;
}

void timer_tick() {
    // Timeout equals 500ms
    if (++sbusTimeout >= 31) {
        sbusTimeout = 31;
        setpoint_source = failsave;
    } else if (++usbTimeout >= 31) {
        usbTimeout = 31;
        if (setpoint_source == flightcomputer) {
            setpoint_source = remote;
        }
    }

    if (setpoint_source == flightcomputer) {
        controller_update(&curr_state, &curr_setpoint, &out_state);
        out_state.motor = curr_setpoint.power;
    } else if (setpoint_source == failsave) {
        out_state.motor = 0;
        out_state.elevon_l = 0;
        out_state.elevon_r = 0;
    } else {
        out_state.motor = NORMALIZE_TARANIS(sbus_latest_data.channel[0]);
        out_state.elevon_l = NORMALIZE_TARANIS(sbus_latest_data.channel[1])- 500;
        out_state.elevon_r = NORMALIZE_TARANIS(sbus_latest_data.channel[2])- 500;
    }
    output_set(&out_state);
    output_led(4, toggle);

    switch (setpoint_source) {
        case failsave:
            output_led(6, off);
            output_led(7, off);
            break;
        case remote:
            output_led(6, off);
            output_led(7, on);
            break;
        case flightcomputer:
            output_led(6, on);
            output_led(7, on);
            break;
        default:
            break;
    }
}

int main(void) {
    cli();
    output_init();
    output_led(0, on);
    output_led(1, MCUSR & (1 << WDRF) ? off : on); // Watchdog
    output_led(2, MCUSR & (1 << BORF) ? off : on); // Brownout
    MCUSR = 0;

    communication_init(&setpoint_update, &sbus_event);
    controller_init(16);
    // Runs at 16.384ms interval, the BNO055 provides data at 100Hz, the output can be updated at 50Hz
    timer0_init(prescaler_1024, &timer_tick);
    sei();

    wdt_enable(WDTO_8S);

    input_init();
    _delay_ms(100);

    uint8_t mux = 0;

    while (true) {
        wdt_reset();
        input_get_state(&curr_state);
        if (++mux >= 10) {
            communication_send_status(&curr_state, &out_state);
            mux = 0;
        }
        communication_handle_usb();
        output_led(0, toggle);
        _delay_ms(10);
    }
}

#pragma clang diagnostic pop
