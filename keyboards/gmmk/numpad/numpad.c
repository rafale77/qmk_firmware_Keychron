/* Copyright 2021 Glorious, LLC <salman@pcgamingrace.com>
 * Modified 2022 by rustedaperture for qmk_firmware
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

#include "quantum.h"

#ifdef RGB_MATRIX_ENABLE

const aw20216s_led_t PROGMEM g_aw20216s_leds[AW20216S_LED_COUNT] = {
/* Refer to AW20216S manual for these locations
 *   driver
 *   |     R location
 *   |     |          G location
 *   |     |          |          B location
 *   |     |          |          | */
    {0, SW1_CS4,  SW1_CS5,  SW1_CS6  },  //  0  NUM
    {0, SW2_CS4,  SW2_CS5,  SW2_CS6  },  //  1  /
    {0, SW1_CS7,  SW1_CS8,  SW1_CS9  },  //  2  *
    {0, SW2_CS7,  SW2_CS8,  SW2_CS9  },  //  3  -
    {0, SW3_CS4,  SW3_CS5,  SW3_CS6  },  //  4  7
    {0, SW4_CS4,  SW4_CS5,  SW4_CS6  },  //  5  8
    {0, SW3_CS7,  SW3_CS8,  SW3_CS9  },  //  6  9
    {0, SW4_CS7,  SW4_CS8,  SW4_CS9  },  //  7  +
    {0, SW5_CS4,  SW5_CS5,  SW5_CS6  },  //  8  4
    {0, SW6_CS4,  SW6_CS5,  SW6_CS6  },  //  9  5
    {0, SW5_CS7,  SW5_CS8,  SW5_CS9  },  //  10 6
    {0, SW7_CS4,  SW7_CS5,  SW7_CS6  },  //  11 1
    {0, SW8_CS4,  SW8_CS5,  SW8_CS6  },  //  12 2
    {0, SW7_CS7,  SW7_CS8,  SW7_CS9  },  //  13 3
    {0, SW8_CS7,  SW8_CS8,  SW8_CS9  },  //  14 ENTER
    {0, SW9_CS4,  SW9_CS5,  SW9_CS6  },  //  15 0
    {0, SW9_CS7,  SW9_CS8,  SW9_CS9  },  //  16 .
    {0, SW1_CS1,  SW1_CS2,  SW1_CS3  },  //  17 LED18
    {0, SW2_CS1,  SW2_CS2,  SW2_CS3  },  //  18 LED19
    {0, SW3_CS1,  SW3_CS2,  SW3_CS3  },  //  19 LED20
    {0, SW4_CS1,  SW4_CS2,  SW4_CS3  },  //  20 LED21
    {0, SW5_CS1,  SW5_CS2,  SW5_CS3  },  //  21 LED22
    {0, SW6_CS1,  SW6_CS2,  SW6_CS3  },  //  22 LED23
    {0, SW7_CS1,  SW7_CS2,  SW7_CS3  },  //  23 LED24
    {0, SW1_CS10, SW1_CS11, SW1_CS12 },  //  24 LED27
    {0, SW2_CS10, SW2_CS11, SW2_CS12 },  //  25 LED28
    {0, SW3_CS10, SW3_CS11, SW3_CS12 },  //  26 LED29
    {0, SW4_CS10, SW4_CS11, SW4_CS12 },  //  27 LED30
    {0, SW5_CS10, SW5_CS11, SW5_CS12 },  //  28 LED31
    {0, SW6_CS10, SW6_CS11, SW6_CS12 },  //  29 LED32
    {0, SW7_CS10, SW7_CS11, SW7_CS12 },  //  30 LED33
};

#    ifdef AW20216S_PW_EN_PIN

void keyboard_pre_init_kb(void) {
    wait_ms(2000);
    gpio_set_pin_output(AW20216S_PW_EN_PIN);
    gpio_write_pin_high(AW20216S_PW_EN_PIN);

    keyboard_pre_init_user();
}
#    endif

// ---- Solid FX + sidelights: saved settings and VIA controls ----
#if defined(ENABLE_RGB_MATRIX_SOLID_SPLASH) || defined(ENABLE_RGB_MATRIX_SOLID_MULTISPLASH) || defined(ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE) || defined(ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE)
#    define NUMPAD_SOLID_FX
#    ifdef VIA_ENABLE
#        include "via.h"
#    endif

extern uint8_t solid_fx_hue, solid_fx_sat, solid_fx_val, solid_fx_fade_pct;

// Sidelight power-on defaults (VIA changes them at runtime). Override in config.h.
#    ifndef NUMPAD_SIDE_HUE
#        define NUMPAD_SIDE_HUE 170 // 0-255 (170 = blue)
#    endif
#    ifndef NUMPAD_SIDE_SAT
#        define NUMPAD_SIDE_SAT 255
#    endif
#    ifndef NUMPAD_SIDE_VAL
#        define NUMPAD_SIDE_VAL 128
#    endif
static uint8_t side_hue    = NUMPAD_SIDE_HUE;
static uint8_t side_sat    = NUMPAD_SIDE_SAT;
static uint8_t side_val    = NUMPAD_SIDE_VAL;
static uint8_t side_enable = 1;

#    define SOLID_FX_MAGIC 0xA6 // bumped: stored layout grew from 5 to 9 bytes

typedef struct {
    uint8_t magic, hue, sat, val, fade_pct, side_enable, side_hue, side_sat, side_val;
} solid_fx_eeprom_t;

_Static_assert((EECONFIG_KB_DATA_SIZE) >= sizeof(solid_fx_eeprom_t), "Define EECONFIG_KB_DATA_SIZE 9 in config.h");

static void solid_fx_save(void) {
    solid_fx_eeprom_t d = {SOLID_FX_MAGIC, solid_fx_hue, solid_fx_sat, solid_fx_val, solid_fx_fade_pct, side_enable, side_hue, side_sat, side_val};
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
    side_enable       = d.side_enable ? 1 : 0;
    side_hue          = d.side_hue;
    side_sat          = d.side_sat;
    side_val          = d.side_val;
}

// Paint the sidelights every frame from their own settings, independent of the effect
bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) {
        return false;
    }
    uint8_t v = side_enable ? side_val : 0;
    if (v > RGB_MATRIX_MAXIMUM_BRIGHTNESS) v = RGB_MATRIX_MAXIMUM_BRIGHTNESS;
    rgb_t rgb = hsv_to_rgb((hsv_t){side_hue, side_sat, v});
    for (uint8_t i = led_min; i < led_max; i++) {
        if (HAS_FLAGS(g_led_config.flags[i], LED_FLAG_UNDERGLOW)) {
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }
    return true;
}

#    ifdef VIA_ENABLE
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data... ]
    if (data[1] == id_custom_channel) {
        uint8_t *v = &data[3];
        if (data[0] == id_custom_save) {
            solid_fx_save();
            return;
        }
        if (data[2] >= 1 && data[2] <= 6) { // 1-3 Solid FX, 4-6 sidelights
            if (data[0] == id_custom_set_value) {
                switch (data[2]) {
                    case 1: solid_fx_hue = v[0]; solid_fx_sat = v[1]; break;
                    case 2: solid_fx_val = v[0]; break;
                    case 3: solid_fx_fade_pct = v[0] > 100 ? 100 : v[0]; break;
                    case 4: side_hue = v[0]; side_sat = v[1]; break;
                    case 5: side_val = v[0]; break;
                    case 6: side_enable = v[0] ? 1 : 0; break;
                }
                return;
            }
            if (data[0] == id_custom_get_value) {
                switch (data[2]) {
                    case 1: v[0] = solid_fx_hue; v[1] = solid_fx_sat; break;
                    case 2: v[0] = solid_fx_val; break;
                    case 3: v[0] = solid_fx_fade_pct; break;
                    case 4: v[0] = side_hue; v[1] = side_sat; break;
                    case 5: v[0] = side_val; break;
                    case 6: v[0] = side_enable; break;
                }
                return;
            }
        }
    }
    data[0] = id_unhandled; // not ours: let VIA's built-in channels handle it
}
#    endif
#endif

void keyboard_post_init_kb(void) {
  #ifdef NUMPAD_SOLID_FX
      solid_fx_load();
      rgb_matrix_set_flags_noeeprom(LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER);
  #endif
    keyboard_post_init_user();
}

#endif
