#if defined(ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE) || defined(ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE)

#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE
RGB_MATRIX_EFFECT(SOLID_REACTIVE_WIDE)
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE
RGB_MATRIX_EFFECT(SOLID_REACTIVE_MULTIWIDE)
#    endif

#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#        ifndef FIXED_BG_BLEND_DEFINED
#            define FIXED_BG_BLEND_DEFINED

// Fixed background in HSV (0-255 scale). Override in config.h.
#            ifndef FIXED_BG_H
#                define FIXED_BG_H 170 // blue
#            endif
#            ifndef FIXED_BG_S
#                define FIXED_BG_S 255
#            endif
#            ifndef FIXED_BG_V
#                define FIXED_BG_V 60
#            endif

// amount: 0 = background, 255 = full splash color.
static inline rgb_t fixed_bg_blend(uint8_t amount, hsv_t sp) {
    int8_t dh = (int8_t)(sp.h - FIXED_BG_H);
    hsv_t  out;
    out.h = (uint8_t)(FIXED_BG_H + ((int16_t)dh * amount) / 255);
    out.s = (uint8_t)(FIXED_BG_S + (((int16_t)sp.s - FIXED_BG_S) * amount) / 255);
    out.v = (uint8_t)(FIXED_BG_V + (((int16_t)sp.v - FIXED_BG_V) * amount) / 255);
    return rgb_matrix_hsv_to_rgb(out);
}
#        endif // FIXED_BG_BLEND_DEFINED

#        ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE
bool SOLID_REACTIVE_WIDE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t count = g_last_hit_tracker.count;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint8_t amount = 0;

        // Wide: most recent press only
        for (uint8_t j = (count > 0 ? count - 1 : 0); j < count; j++) {
            int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint8_t  dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed, 1));
            uint16_t eff  = tick + dist * 5;
            if (eff > 255) eff = 255;
            amount = 255 - eff;
        }

        rgb_t c = fixed_bg_blend(amount, rgb_matrix_config.hsv);
        rgb_matrix_set_color(i, c.r, c.g, c.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}
#        endif // ENABLE_RGB_MATRIX_SOLID_REACTIVE_WIDE

#        ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE
bool SOLID_REACTIVE_MULTIWIDE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    uint8_t count = g_last_hit_tracker.count;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint8_t amount = 0;

        // Multiwide: all recent presses, glows add up
        for (uint8_t j = 0; j < count; j++) {
            int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint8_t  dist = sqrt16(dx * dx + dy * dy);
            uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed, 1));
            uint16_t eff  = tick + dist * 5;
            if (eff > 255) eff = 255;
            amount = qadd8(amount, 255 - eff);
        }

        rgb_t c = fixed_bg_blend(amount, rgb_matrix_config.hsv);
        rgb_matrix_set_color(i, c.r, c.g, c.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}
#        endif // ENABLE_RGB_MATRIX_SOLID_REACTIVE_MULTIWIDE

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif
