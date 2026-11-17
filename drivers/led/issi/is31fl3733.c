/* Copyright 2017 Jason Williams
 * Copyright 2018 Jack Humbert
 * Copyright 2018 Yiancar
 * Copyright 2021 Doni Crosby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "is31fl3733.h"
#include "i2c_master.h"
#include "gpio.h"
#include "wait.h"

#define IS31FL3733_PWM_REGISTER_COUNT 192
#define IS31FL3733_LED_CONTROL_REGISTER_COUNT 24

#ifndef IS31FL3733_I2C_TIMEOUT
#    define IS31FL3733_I2C_TIMEOUT 100
#endif

#ifndef IS31FL3733_I2C_PERSISTENCE
#    define IS31FL3733_I2C_PERSISTENCE 0
#endif

#ifndef IS31FL3733_PWM_FREQUENCY
#    define IS31FL3733_PWM_FREQUENCY IS31FL3733_PWM_FREQUENCY_8K4_HZ // PFS - IS31FL3733B only
#endif

#ifndef IS31FL3733_SW_PULLUP
#    define IS31FL3733_SW_PULLUP IS31FL3733_PUR_0_OHM
#endif

#ifndef IS31FL3733_CS_PULLDOWN
#    define IS31FL3733_CS_PULLDOWN IS31FL3733_PDR_0_OHM
#endif

#ifndef IS31FL3733_GLOBAL_CURRENT
#    define IS31FL3733_GLOBAL_CURRENT 0xFF
#endif

#ifndef IS31FL3733_SYNC_1
#    define IS31FL3733_SYNC_1 IS31FL3733_SYNC_NONE
#endif
#ifndef IS31FL3733_SYNC_2
#    define IS31FL3733_SYNC_2 IS31FL3733_SYNC_NONE
#endif
#ifndef IS31FL3733_SYNC_3
#    define IS31FL3733_SYNC_3 IS31FL3733_SYNC_NONE
#endif
#ifndef IS31FL3733_SYNC_4
#    define IS31FL3733_SYNC_4 IS31FL3733_SYNC_NONE
#endif

const uint8_t i2c_addresses[IS31FL3733_DRIVER_COUNT] = {
    IS31FL3733_I2C_ADDRESS_1,
#ifdef IS31FL3733_I2C_ADDRESS_2
    IS31FL3733_I2C_ADDRESS_2,
#    ifdef IS31FL3733_I2C_ADDRESS_3
    IS31FL3733_I2C_ADDRESS_3,
#        ifdef IS31FL3733_I2C_ADDRESS_4
    IS31FL3733_I2C_ADDRESS_4,
#        endif
#    endif
#endif
};

const uint8_t driver_sync[IS31FL3733_DRIVER_COUNT] = {
    IS31FL3733_SYNC_1,
#ifdef IS31FL3733_I2C_ADDRESS_2
    IS31FL3733_SYNC_2,
#    ifdef IS31FL3733_I2C_ADDRESS_3
    IS31FL3733_SYNC_3,
#        ifdef IS31FL3733_I2C_ADDRESS_4
    IS31FL3733_SYNC_4,
#        endif
#    endif
#endif
};

typedef struct is31fl3733_driver_t {
    uint8_t  pwm_buffer[IS31FL3733_PWM_REGISTER_COUNT];
    bool     pwm_buffer_dirty;
    uint16_t pwm_block_dirty; // bit n set = the 16-byte PWM block for scan row n+1 has changed
    uint8_t  led_control_buffer[IS31FL3733_LED_CONTROL_REGISTER_COUNT];
    bool     led_control_buffer_dirty;
} PACKED is31fl3733_driver_t;

is31fl3733_driver_t driver_buffers[IS31FL3733_DRIVER_COUNT] = {{
    .pwm_buffer               = {0},
    .pwm_buffer_dirty         = false,
    .pwm_block_dirty          = 0,
    .led_control_buffer       = {0},
    .led_control_buffer_dirty = false,
}};

void is31fl3733_write_register(uint8_t index, uint8_t reg, uint8_t data) {
#if IS31FL3733_I2C_PERSISTENCE > 0
    for (uint8_t i = 0; i < IS31FL3733_I2C_PERSISTENCE; i++) {
        if (i2c_write_register(i2c_addresses[index] << 1, reg, &data, 1, IS31FL3733_I2C_TIMEOUT) == I2C_STATUS_SUCCESS) break;
    }
#else
    i2c_write_register(i2c_addresses[index] << 1, reg, &data, 1, IS31FL3733_I2C_TIMEOUT);
#endif
}

void is31fl3733_select_page(uint8_t index, uint8_t page) {
    is31fl3733_write_register(index, IS31FL3733_REG_COMMAND_WRITE_LOCK, IS31FL3733_COMMAND_WRITE_LOCK_MAGIC);
    is31fl3733_write_register(index, IS31FL3733_REG_COMMAND, page);
}

// Write 'length' bytes starting at register 'reg'. The chip auto-increments the register address.
// Returns true on success; honours IS31FL3733_I2C_PERSISTENCE retries.
static bool is31fl3733_write_block(uint8_t index, uint8_t reg, const uint8_t *data, uint8_t length) {
#if IS31FL3733_I2C_PERSISTENCE > 0
    for (uint8_t i = 0; i < IS31FL3733_I2C_PERSISTENCE; i++) {
        if (i2c_write_register(i2c_addresses[index] << 1, reg, (uint8_t *)data, length, IS31FL3733_I2C_TIMEOUT) == I2C_STATUS_SUCCESS) return true;
    }
    return false;
#else
    return i2c_write_register(i2c_addresses[index] << 1, reg, (uint8_t *)data, length, IS31FL3733_I2C_TIMEOUT) == I2C_STATUS_SUCCESS;
#endif
}

void is31fl3733_write_pwm_buffer(uint8_t index) {
    // Assumes page 1 is already selected. Each 16-byte block is one scan row; send only changed blocks.
    uint16_t pending = driver_buffers[index].pwm_block_dirty;

    for (uint8_t block = 0; block < IS31FL3733_PWM_REGISTER_COUNT / 16; block++) {
        uint16_t bit = (uint16_t)1 << block;
        if (!(pending & bit)) continue;
        uint8_t reg = block * 16;
        if (is31fl3733_write_block(index, reg, driver_buffers[index].pwm_buffer + reg, 16)) pending &= ~bit;
    }
    driver_buffers[index].pwm_block_dirty = pending;
}

void is31fl3733_init_drivers(void) {
    i2c_init();

#if defined(IS31FL3733_SDB_PIN)
    gpio_set_pin_output(IS31FL3733_SDB_PIN);
    gpio_write_pin_high(IS31FL3733_SDB_PIN);
#endif

    for (uint8_t i = 0; i < IS31FL3733_DRIVER_COUNT; i++) {
        is31fl3733_init(i);
    }

    for (int i = 0; i < IS31FL3733_LED_COUNT; i++) {
        is31fl3733_set_led_control_register(i, true, true, true);
    }

    for (uint8_t i = 0; i < IS31FL3733_DRIVER_COUNT; i++) {
        is31fl3733_update_led_control_registers(i);
    }
}

void is31fl3733_init(uint8_t index) {
    // In order to avoid the LEDs being driven with garbage data
    // in the LED driver's PWM registers, shutdown is enabled last.
    // Set up the mode and other settings, clear the PWM registers,
    // then disable software shutdown.

    static const uint8_t zeros[IS31FL3733_LED_CONTROL_REGISTER_COUNT] = {0};
    is31fl3733_select_page(index, IS31FL3733_COMMAND_LED_CONTROL);

    // Turn off all LEDs. one 24-byte transfer instead of 24 single-byte writes.
    is31fl3733_write_block(index, 0x00, zeros, IS31FL3733_LED_CONTROL_REGISTER_COUNT);

    is31fl3733_select_page(index, IS31FL3733_COMMAND_PWM);


    // Set PWM on all LEDs to 0: 12 transfers of 16 bytes instead of 192 single-byte writes.
    // No need to setup Breath registers to PWM as that is the default.
    for (uint8_t reg = 0; reg < IS31FL3733_PWM_REGISTER_COUNT; reg += 16) {
        is31fl3733_write_block(index, reg, zeros, 16);
    }

    is31fl3733_select_page(index, IS31FL3733_COMMAND_FUNCTION);

    uint8_t sync = driver_sync[index];

    // Set de-ghost pull-up resistors (SWx)
    is31fl3733_write_register(index, IS31FL3733_FUNCTION_REG_SW_PULLUP, IS31FL3733_SW_PULLUP);
    // Set de-ghost pull-down resistors (CSx)
    is31fl3733_write_register(index, IS31FL3733_FUNCTION_REG_CS_PULLDOWN, IS31FL3733_CS_PULLDOWN);
    // Set global current to maximum.
    is31fl3733_write_register(index, IS31FL3733_FUNCTION_REG_GLOBAL_CURRENT, IS31FL3733_GLOBAL_CURRENT);
    // Disable software shutdown.
    is31fl3733_write_register(index, IS31FL3733_FUNCTION_REG_CONFIGURATION, ((sync & 0b11) << 6) | ((IS31FL3733_PWM_FREQUENCY & 0b111) << 3) | 0x01);

    // Wait 10ms to ensure the device has woken up.
    wait_ms(10);
}

// Optional per-board color correction; the default does nothing
__attribute__((weak)) void is31fl3733_trim_color(int index, uint8_t *red, uint8_t *green, uint8_t *blue) {}

void is31fl3733_set_color(int index, uint8_t red, uint8_t green, uint8_t blue) {
    is31fl3733_trim_color(index, &red, &green, &blue); // per-board color correction (keep)

    if (index >= 0 && index < IS31FL3733_LED_COUNT) {
        is31fl3733_led_t led;
        memcpy_P(&led, (&g_is31fl3733_leds[index]), sizeof(led));

        is31fl3733_driver_t *drv   = &driver_buffers[led.driver];
        uint16_t             dirty = 0;
        if (drv->pwm_buffer[led.r] != red)   { drv->pwm_buffer[led.r] = red;   dirty |= (uint16_t)1 << (led.r / 16); }
        if (drv->pwm_buffer[led.g] != green) { drv->pwm_buffer[led.g] = green; dirty |= (uint16_t)1 << (led.g / 16); }
        if (drv->pwm_buffer[led.b] != blue)  { drv->pwm_buffer[led.b] = blue;  dirty |= (uint16_t)1 << (led.b / 16); }
        if (dirty) {
            drv->pwm_block_dirty |= dirty;
            drv->pwm_buffer_dirty = true;
        }
    }
}

void is31fl3733_set_color_all(uint8_t red, uint8_t green, uint8_t blue) {
    for (int i = 0; i < IS31FL3733_LED_COUNT; i++) {
        is31fl3733_set_color(i, red, green, blue);
    }
}

void is31fl3733_set_led_control_register(uint8_t index, bool red, bool green, bool blue) {
    is31fl3733_led_t led;
    memcpy_P(&led, (&g_is31fl3733_leds[index]), sizeof(led));

    uint8_t control_register_r = led.r / 8;
    uint8_t control_register_g = led.g / 8;
    uint8_t control_register_b = led.b / 8;
    uint8_t bit_r              = led.r % 8;
    uint8_t bit_g              = led.g % 8;
    uint8_t bit_b              = led.b % 8;

    if (red) {
        driver_buffers[led.driver].led_control_buffer[control_register_r] |= (1 << bit_r);
    } else {
        driver_buffers[led.driver].led_control_buffer[control_register_r] &= ~(1 << bit_r);
    }
    if (green) {
        driver_buffers[led.driver].led_control_buffer[control_register_g] |= (1 << bit_g);
    } else {
        driver_buffers[led.driver].led_control_buffer[control_register_g] &= ~(1 << bit_g);
    }
    if (blue) {
        driver_buffers[led.driver].led_control_buffer[control_register_b] |= (1 << bit_b);
    } else {
        driver_buffers[led.driver].led_control_buffer[control_register_b] &= ~(1 << bit_b);
    }

    driver_buffers[led.driver].led_control_buffer_dirty = true;
}

void is31fl3733_update_pwm_buffers(uint8_t index) {
    if (driver_buffers[index].pwm_buffer_dirty) {
        is31fl3733_select_page(index, IS31FL3733_COMMAND_PWM);
        is31fl3733_write_pwm_buffer(index);
        driver_buffers[index].pwm_buffer_dirty = (driver_buffers[index].pwm_block_dirty != 0);
    }
}

void is31fl3733_update_led_control_registers(uint8_t index) {
    if (driver_buffers[index].led_control_buffer_dirty) {
        is31fl3733_select_page(index, IS31FL3733_COMMAND_LED_CONTROL);
        is31fl3733_write_block(index, 0x00, driver_buffers[index].led_control_buffer, IS31FL3733_LED_CONTROL_REGISTER_COUNT);
        driver_buffers[index].led_control_buffer_dirty = false;
    }
}

void is31fl3733_flush(void) {
    for (uint8_t i = 0; i < IS31FL3733_DRIVER_COUNT; i++) {
        is31fl3733_update_pwm_buffers(i);
    }
}
