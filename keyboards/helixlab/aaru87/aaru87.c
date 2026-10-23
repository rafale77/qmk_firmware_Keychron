// Copyright 2022 luantty2 (@luantty2)
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H

#if defined(RGB_MATRIX_ENABLE) && (defined(ENABLE_RGB_MATRIX_SOLID_SPLASH) || defined(ENABLE_RGB_MATRIX_SOLID_MULTISPLASH) || defined(ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE) || defined(ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE))
#    define AARU87_SOLID_FX
#    ifdef VIA_ENABLE
#        include "via.h"
#    endif
extern uint8_t solid_fx_hue, solid_fx_sat, solid_fx_val, solid_fx_fade_pct;

#define SOLID_FX_MAGIC 0xA5

typedef struct {
    uint8_t magic, hue, sat, val, fade_pct;
} solid_fx_eeprom_t;

static void solid_fx_save(void) {
    solid_fx_eeprom_t d = {SOLID_FX_MAGIC, solid_fx_hue, solid_fx_sat, solid_fx_val, solid_fx_fade_pct};
    eeconfig_update_kb_datablock(&d, 0, sizeof(d));
}

static void solid_fx_load(void) {
    solid_fx_eeprom_t d;
    eeconfig_read_kb_datablock(&d, 0, sizeof(d));
    if (d.magic != SOLID_FX_MAGIC) { // blank or stale EEPROM: keep compile-time defaults and store them
        solid_fx_save();
        return;
    }
    solid_fx_hue      = d.hue;
    solid_fx_sat      = d.sat;
    solid_fx_val      = d.val;
    solid_fx_fade_pct = d.fade_pct > 100 ? 100 : d.fade_pct;
}

// Called when EEPROM is reset: writes the compile-time defaults
void eeconfig_init_kb_datablock(void) {
    solid_fx_save();
}
#endif

#ifdef RGB_MATRIX_ENABLE
#    include "rgb_matrix_effects_custom.h"
#endif

#ifdef RGB_MATRIX_ENABLE
extern bool fade_completed;
#endif

void keyboard_post_init_kb(void) {
    debug_enable   = false;
    debug_matrix   = false;
    debug_keyboard = false;
    debug_mouse    = false;
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_set_target_val();
#endif
    wait_ms(100);
    #ifdef AARU87_SOLID_FX
      solid_fx_load();
    #endif
    keyboard_post_init_user();
}

void matrix_scan_kb(void) {
#ifdef RGB_MATRIX_ENABLE
    if (!fade_completed) {
        if (rgb_matrix_fade_in()) {
            fade_completed = true;
        }
    }
#endif
    matrix_scan_user();
}

#ifdef RGB_MATRIX_ENABLE
const is31fl3733_led_t PROGMEM g_is31fl3733_leds[IS31FL3733_LED_COUNT] = {
    /* driver, R, G, B */
    {0, SW11_CS1, SW12_CS1, SW10_CS1},
    {0, SW11_CS2, SW12_CS2, SW10_CS2},
    {0, SW11_CS3, SW12_CS3, SW10_CS3},
    {0, SW11_CS4, SW12_CS4, SW10_CS4},
    {0, SW11_CS5, SW12_CS5, SW10_CS5},
    {0, SW11_CS6, SW12_CS6, SW10_CS6},
    {0, SW11_CS7, SW12_CS7, SW10_CS7},
    {0, SW11_CS8, SW12_CS8, SW10_CS8},
    {0, SW11_CS9, SW12_CS9, SW10_CS9},
    {0, SW11_CS10, SW12_CS10, SW10_CS10},
    {0, SW11_CS11, SW12_CS11, SW10_CS11},
    {0, SW11_CS12, SW12_CS12, SW10_CS12},
    {0, SW11_CS13, SW12_CS13, SW10_CS13},
    {0, SW11_CS14, SW12_CS14, SW10_CS14},
    {0, SW11_CS15, SW12_CS15, SW10_CS15},
    {0, SW11_CS16, SW12_CS16, SW10_CS16},

    /*row1*/
    {0, SW8_CS1, SW9_CS1, SW7_CS1},
    {0, SW8_CS2, SW9_CS2, SW7_CS2},
    {0, SW8_CS3, SW9_CS3, SW7_CS3},
    {0, SW8_CS4, SW9_CS4, SW7_CS4},
    {0, SW8_CS5, SW9_CS5, SW7_CS5},
    {0, SW8_CS6, SW9_CS6, SW7_CS6},
    {0, SW8_CS7, SW9_CS7, SW7_CS7},
    {0, SW8_CS8, SW9_CS8, SW7_CS8},
    {0, SW8_CS9, SW9_CS9, SW7_CS9},
    {0, SW8_CS10, SW9_CS10, SW7_CS10},
    {0, SW8_CS11, SW9_CS11, SW7_CS11},
    {0, SW8_CS12, SW9_CS12, SW7_CS12},
    {0, SW8_CS13, SW9_CS13, SW7_CS13},
    {0, SW8_CS14, SW9_CS14, SW7_CS14},
    {0, SW8_CS15, SW9_CS15, SW7_CS15},
    {0, SW8_CS16, SW9_CS16, SW7_CS16},
    {1, SW2_CS13, SW3_CS13, SW1_CS13},

    /*row2*/
    {0, SW5_CS1, SW6_CS1, SW4_CS1},
    {0, SW5_CS2, SW6_CS2, SW4_CS2},
    {0, SW5_CS3, SW6_CS3, SW4_CS3},
    {0, SW5_CS4, SW6_CS4, SW4_CS4},
    {0, SW5_CS5, SW6_CS5, SW4_CS5},
    {0, SW5_CS6, SW6_CS6, SW4_CS6},
    {0, SW5_CS7, SW6_CS7, SW4_CS7},
    {0, SW5_CS8, SW6_CS8, SW4_CS8},
    {0, SW5_CS9, SW6_CS9, SW4_CS9},
    {0, SW5_CS10, SW6_CS10, SW4_CS10},
    {0, SW5_CS11, SW6_CS11, SW4_CS11},
    {0, SW5_CS12, SW6_CS12, SW4_CS12},
    {0, SW5_CS13, SW6_CS13, SW4_CS13},
    {0, SW5_CS14, SW6_CS14, SW4_CS14},
    {0, SW5_CS15, SW6_CS15, SW4_CS15},
    {0, SW5_CS16, SW6_CS16, SW4_CS16},
    {1, SW2_CS12, SW3_CS12, SW1_CS12},

    /*row3*/
    {0, SW2_CS1, SW3_CS1, SW1_CS1},
    {0, SW2_CS2, SW3_CS2, SW1_CS2},
    {0, SW2_CS3, SW3_CS3, SW1_CS3},
    {0, SW2_CS4, SW3_CS4, SW1_CS4},
    {0, SW2_CS5, SW3_CS5, SW1_CS5},
    {0, SW2_CS6, SW3_CS6, SW1_CS6},
    {0, SW2_CS7, SW3_CS7, SW1_CS7},
    {0, SW2_CS8, SW3_CS8, SW1_CS8},
    {0, SW2_CS9, SW3_CS9, SW1_CS9},
    {0, SW2_CS10, SW3_CS10, SW1_CS10},
    {0, SW2_CS11, SW3_CS11, SW1_CS11},
    {0, SW2_CS12, SW3_CS12, SW1_CS12},
    {0, SW2_CS13, SW3_CS13, SW1_CS13},

    /*row4*/
    {1, SW5_CS1, SW6_CS1, SW4_CS1},
    {1, SW5_CS2, SW6_CS2, SW4_CS2},
    {1, SW5_CS3, SW6_CS3, SW4_CS3},
    {1, SW5_CS4, SW6_CS4, SW4_CS4},
    {1, SW5_CS5, SW6_CS5, SW4_CS5},
    {1, SW5_CS6, SW6_CS6, SW4_CS6},
    {1, SW5_CS7, SW6_CS7, SW4_CS7},
    {1, SW5_CS8, SW6_CS8, SW4_CS8},
    {1, SW5_CS14, SW6_CS14, SW4_CS14},
    {1, SW5_CS15, SW6_CS15, SW4_CS15},
    {1, SW5_CS16, SW6_CS16, SW4_CS16},
    {1, SW5_CS12, SW6_CS12, SW4_CS12},
    {1, SW5_CS13, SW6_CS13, SW4_CS13},

    /*row5*/
    {1, SW2_CS1, SW3_CS1, SW1_CS1},
    {1, SW2_CS2, SW3_CS2, SW1_CS2},
    {1, SW2_CS3, SW3_CS3, SW1_CS3},
    {1, SW2_CS4, SW3_CS4, SW1_CS4},
    {1, SW2_CS5, SW3_CS5, SW1_CS5},
    {1, SW2_CS6, SW3_CS6, SW1_CS6},
    {1, SW2_CS7, SW3_CS7, SW1_CS7},
    {1, SW2_CS8, SW3_CS8, SW1_CS8},
    {1, SW2_CS14, SW3_CS14, SW1_CS14},
    {1, SW2_CS15, SW3_CS15, SW1_CS15},
    {1, SW2_CS16, SW3_CS16, SW1_CS16},
};

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) {
        return false;
    }

    if (host_keyboard_led_state().caps_lock) {
        hsv_t hsv = rgb_matrix_get_hsv();
        if (hsv.h % 191 > 0) {
            hsv.h -= 191;
        }
        hsv.h += 64;
        rgb_t rgb = hsv_to_rgb(hsv);
        for (uint8_t i = led_min; i < led_max; i++) {
            if (HAS_FLAGS(g_led_config.flags[i], LED_FLAG_INDICATOR)) {
                rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
            }
        }
    }
    return false;
}
#endif

void eeconfig_init_kb(void) {
#ifdef RGBLIGHT_ENABLE
    rgblight_enable();
    rgblight_sethsv(45, 255, 255);
    rgblight_mode(RGBLIGHT_MODE_BREATHING + 1);
#endif
    eeconfig_init_user();
}
#if defined(AARU87_SOLID_FX) && defined(VIA_ENABLE)
enum {
    id_solid_fx_color      = 1, // hue, sat
    id_solid_fx_brightness = 2,
    id_solid_fx_fade       = 3,
};

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data... ]
    uint8_t *command_id = &(data[0]);
    uint8_t *channel_id = &(data[1]);
    uint8_t *value_id   = &(data[2]);
    uint8_t *value_data = &(data[3]);

    if (*channel_id != id_custom_channel) {
        *command_id = id_unhandled;
        return;
    }

    switch (*command_id) {
        case id_custom_set_value:
            switch (*value_id) {
                case id_solid_fx_color:
                    solid_fx_hue = value_data[0];
                    solid_fx_sat = value_data[1];
                    break;
                case id_solid_fx_brightness:
                    solid_fx_val = value_data[0];
                    break;
                case id_solid_fx_fade:
                    solid_fx_fade_pct = value_data[0] > 100 ? 100 : value_data[0];
                    break;
                default:
                    *command_id = id_unhandled;
                    break;
            }
            break;
        case id_custom_get_value:
            switch (*value_id) {
                case id_solid_fx_color:
                    value_data[0] = solid_fx_hue;
                    value_data[1] = solid_fx_sat;
                    break;
                case id_solid_fx_brightness:
                    value_data[0] = solid_fx_val;
                    break;
                case id_solid_fx_fade:
                    value_data[0] = solid_fx_fade_pct;
                    break;
                default:
                    *command_id = id_unhandled;
                    break;
            }
            break;
        case id_custom_save:
            solid_fx_save();
            break;
        default:
            *command_id = id_unhandled;
            break;
    }
}
#endif
