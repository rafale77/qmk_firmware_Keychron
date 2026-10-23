/* Copyright 2025 ~ 2026 @ Keychron (https://www.keychron.com)
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

#include "keychron.h"
// ---- Solid FX: saved color settings + VIA controls ----
#if defined(RGB_MATRIX_ENABLE) && (defined(ENABLE_RGB_MATRIX_SOLID_SPLASH) || defined(ENABLE_RGB_MATRIX_SOLID_MULTISPLASH) || defined(ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE) || defined(ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE))
#    define P3_HE_SOLID_FX
#    ifdef VIA_ENABLE
#        include "via.h"
#    endif

extern uint8_t solid_fx_hue, solid_fx_sat, solid_fx_val, solid_fx_fade_pct;

#    define SOLID_FX_MAGIC 0xA5

typedef struct {
    uint8_t magic, hue, sat, val, fade_pct;
} solid_fx_eeprom_t;

_Static_assert((EECONFIG_USER_DATA_SIZE) >= sizeof(solid_fx_eeprom_t), "Define EECONFIG_USER_DATA_SIZE 5 in config.h");

static void solid_fx_save(void) {
    // Datablock writes only work while the stored version matches, so make sure it does.
    solid_fx_eeprom_t d = {SOLID_FX_MAGIC, solid_fx_hue, solid_fx_sat, solid_fx_val, solid_fx_fade_pct};
    eeconfig_update_user_datablock(&d, 0, sizeof(d));
}

static void solid_fx_load(void) {
    solid_fx_eeprom_t d;
    eeconfig_read_user_datablock(&d, 0, sizeof(d));
    if (d.magic != SOLID_FX_MAGIC) { // first run or reset: keep compile-time defaults and store them
        solid_fx_save();
        return;
    }
    solid_fx_hue      = d.hue;
    solid_fx_sat      = d.sat;
    solid_fx_val      = d.val;
    solid_fx_fade_pct = d.fade_pct > 100 ? 100 : d.fade_pct;
}

#    ifdef VIA_ENABLE
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data... ]
    if (data[1] == id_custom_channel) {
        uint8_t *value_data = &data[3];
        if (data[0] == id_custom_save) {
            solid_fx_save();
            return;
        }
        if (data[2] >= 1 && data[2] <= 3) { // 1 = color, 2 = brightness, 3 = fade
            if (data[0] == id_custom_set_value) {
                if (data[2] == 1) { solid_fx_hue = value_data[0]; solid_fx_sat = value_data[1]; }
                else if (data[2] == 2) { solid_fx_val = value_data[0]; }
                else { solid_fx_fade_pct = value_data[0] > 100 ? 100 : value_data[0]; }
                return;
            }
            if (data[0] == id_custom_get_value) {
                if (data[2] == 1) { value_data[0] = solid_fx_hue; value_data[1] = solid_fx_sat; }
                else if (data[2] == 2) { value_data[0] = solid_fx_val; }
                else { value_data[0] = solid_fx_fade_pct; }
                return;
            }
        }
    }
    data[0] = id_unhandled; // not ours: let VIA's built-in channels (brightness, effect, color) handle it
}
#    endif
#endif

void keyboard_post_init_kb(void) {
    keychron_common_init();
    #ifdef P3_HE_SOLID_FX
      solid_fx_load();
    #endif
    keyboard_post_init_user();

}

#ifdef LK_WIRELESS_ENABLE
bool lpm_is_kb_idle(void) {
    return !backlight_indicator_is_active();
}
#endif
