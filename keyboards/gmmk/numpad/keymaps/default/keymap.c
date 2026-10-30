/* Copyright 2021 Glorious, LLC <salman@pcgamingrace.com>
 * Modified 2022 by rustedaperture for qmk_firmware

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H
#include "analog.h"
#include "qmk_midi.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

//      NUM      /       *       -
//      7        8       9       +
//      4        5       6       ENCODER
//      1        2       3       RET
//      0                        .

  [0] = LAYOUT(
    MO(1),   KC_PSLS,   KC_PAST,   KC_PMNS,
    KC_P7,   KC_P8,     KC_P9,     KC_PPLS,
    KC_P4,   KC_P5,     KC_P6,     RM_TOGG,
    KC_P1,   KC_P2,     KC_P3,     KC_PENT,
    KC_P0,                         KC_PDOT
  ),
  [1] = LAYOUT(
    _______,   KC_PSLS,   KC_PAST,   KC_PMNS,
    KC_P7,     RM_VALU,   KC_P9,     KC_PPLS,
    RM_PREV,   KC_P5,     RM_NEXT,   KC_CALC,
    KC_P1,     RM_VALD,   KC_P3,     KC_PENT,
    RM_TOGG,                         QK_BOOT
  )
};

// Potentiometer Slider, MIDI Control

uint8_t divisor = 0;

#define SLIDER_MIN_READING 8     // reading with the slider at the bottom stop (measure yours)
#define SLIDER_MAX_READING 950  // reading at the top stop; keep it a few counts BELOW the real maximum
#define SLIDER_HYSTERESIS  4     // brightness units a move must exceed to be applied (limits flicker)

void slider(void) {
    if (divisor++) { /* only run the slider function 1/256 times it's called */
        return;
    }

    static int16_t last_val = -1; // brightness the slider last applied; -1 = not synced yet

    uint16_t reading = analogReadPin(SLIDER_PIN);
    if (reading < SLIDER_MIN_READING) reading = SLIDER_MIN_READING;
    if (reading > SLIDER_MAX_READING) reading = SLIDER_MAX_READING;

    // Position -> brightness, exactly 0 at the bottom and exactly the board's maximum at the top
    uint8_t val = ((uint32_t)(reading - SLIDER_MIN_READING) * RGB_MATRIX_MAXIMUM_BRIGHTNESS) / (SLIDER_MAX_READING - SLIDER_MIN_READING);

    if (last_val < 0) { // first read after boot: remember the position, keep the saved brightness
        last_val = val;
        return;
    }

    bool at_end = (val == 0 || val == RGB_MATRIX_MAXIMUM_BRIGHTNESS);
    if (abs((int16_t)val - last_val) >= SLIDER_HYSTERESIS || (at_end && val != last_val)) {
        last_val  = val;
        hsv_t hsv = rgb_matrix_get_hsv();
        rgb_matrix_sethsv(hsv.h, hsv.s, val);
    }
}

void housekeeping_task_user(void) {
    slider();
}

// Add encoder functionality for scrolling
#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = { ENCODER_CCW_CW(KC_VOLU, KC_VOLD) },
    [1] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    // ...one line for every layer in your keymap
};
#endif
