#if defined(ENABLE_RGB_MATRIX_SOLID_SPLASH) || defined(ENABLE_RGB_MATRIX_SOLID_MULTISPLASH)

#    ifdef ENABLE_RGB_MATRIX_SOLID_SPLASH
RGB_MATRIX_EFFECT(SOLID_SPLASH)
#    endif
#    ifdef ENABLE_RGB_MATRIX_SOLID_MULTISPLASH
RGB_MATRIX_EFFECT(SOLID_MULTISPLASH)
#    endif

#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// ---- Shared helpers: keep this block identical in solid_splash.h and solid_reactive_wide.h ----
#        ifndef SOLID_FX_HELPERS_DEFINED
#            define SOLID_FX_HELPERS_DEFINED

// Power-on defaults (VIA can change them at runtime). Override in config.h.
#            ifndef SOLID_FX_HUE_DEG
#                define SOLID_FX_HUE_DEG 60 // 0-360
#            endif
#            ifndef SOLID_FX_SAT_PCT
#                define SOLID_FX_SAT_PCT 100 // 0-100
#            endif
#            ifndef SOLID_FX_VAL_PCT
#                define SOLID_FX_VAL_PCT 100 // 0-100
#            endif
#            ifndef SOLID_FX_FADE_PCT
#                define SOLID_FX_FADE_PCT 20 // share of the animation spent fading out
#            endif
#            ifndef SOLID_FX_LENGTH
#                define SOLID_FX_LENGTH 255 // animation lifetime in ticks (stock = 255)
#            endif

// Runtime values. Hue/sat/val use QMK's 0-255 scale, fade is 0-100.
// Not static: keyboard code (VIA handler, EEPROM) accesses them with extern.
uint8_t solid_fx_hue      = (uint8_t)((((uint32_t)(SOLID_FX_HUE_DEG % 360) * 255UL) + 180UL) / 360UL);
uint8_t solid_fx_sat      = (uint8_t)((((uint32_t)(SOLID_FX_SAT_PCT) * 255UL) + 50UL) / 100UL);
uint8_t solid_fx_val      = (uint8_t)((((uint32_t)(SOLID_FX_VAL_PCT) * 255UL) + 50UL) / 100UL);
uint8_t solid_fx_fade_pct = SOLID_FX_FADE_PCT;

// age: ticks since the animation reached this LED. 255 during the hold, ramps to 0 over the
// last solid_fx_fade_pct percent of the lifetime, then 0.
static inline uint8_t solid_fx_amount(uint32_t age) {
    if (age >= SOLID_FX_LENGTH) return 0;
    uint32_t fade_len = ((uint32_t)SOLID_FX_LENGTH * solid_fx_fade_pct) / 100UL;
    if (fade_len == 0 || age < (uint32_t)SOLID_FX_LENGTH - fade_len) return 255;
    return (uint8_t)((((uint32_t)SOLID_FX_LENGTH - age) * 255UL) / fade_len);
}

// Blend from the live background color to the animation color in HSV, shortest hue path.
// amount: 0 = background, 255 = animation color.
static inline rgb_t solid_fx_blend(uint8_t amount, hsv_t bg) {
    int8_t dh = (int8_t)(solid_fx_hue - bg.h);
    hsv_t  out;
    out.h = (uint8_t)(bg.h + ((int32_t)dh * amount) / 255);
    out.s = (uint8_t)(bg.s + (((int32_t)solid_fx_sat - bg.s) * amount) / 255);
    out.v = (uint8_t)(bg.v + (((int32_t)solid_fx_val - bg.v) * amount) / 255);
    return rgb_matrix_hsv_to_rgb(out);
}
#        endif // SOLID_FX_HELPERS_DEFINED
// ---- end shared helpers ----sg

// Splash ring size (hard-coded). Smaller = smaller ring. Override in config.h.
// Width: how long each LED stays lit after the wave passes it (ring thickness).
#        ifndef SOLID_SPLASH_RING_WIDTH
#            define SOLID_SPLASH_RING_WIDTH 130
#        endif
// Radius: the ring stops expanding at this distance, in LED-coordinate units (a key is about 12).
#        ifndef SOLID_SPLASH_MAX_RADIUS
#            define SOLID_SPLASH_MAX_RADIUS 250
#        endif

#        ifdef ENABLE_RGB_MATRIX_SOLID_SPLASH

bool SOLID_SPLASH(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    const hsv_t bg_hsv = rgb_matrix_config.hsv; // background follows hue/sat/brightness keys
    const rgb_t bg_rgb = rgb_matrix_hsv_to_rgb(bg_hsv);
    uint8_t     count  = g_last_hit_tracker.count;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint8_t amount = 0;

        // Splash: most recent press only
        for (uint8_t j = (count > 0 ? count - 1 : 0); j < count; j++) {
            int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint8_t  dist = sqrt16(dx * dx + dy * dy);
            if (dist > SOLID_SPLASH_MAX_RADIUS) continue;

            uint16_t tick   = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed, 1));
            uint16_t behind = tick - dist; // wraps to a huge value until the wave arrives
            // Stretch the short ring lifetime onto the shared hold/fade curve
            uint32_t age = ((uint32_t)behind * SOLID_FX_LENGTH) / SOLID_SPLASH_RING_WIDTH;
            uint8_t  a   = solid_fx_amount(age);
            if (a > amount) amount = a;
        }

        rgb_t c = amount ? solid_fx_blend(amount, bg_hsv) : bg_rgb;
        rgb_matrix_set_color(i, c.r, c.g, c.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}
#        endif // ENABLE_RGB_MATRIX_SOLID_SPLASH

#        ifdef ENABLE_RGB_MATRIX_SOLID_MULTISPLASH
bool SOLID_MULTISPLASH(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    const hsv_t bg_hsv = rgb_matrix_config.hsv;
    const rgb_t bg_rgb = rgb_matrix_hsv_to_rgb(bg_hsv);
    uint8_t     count  = g_last_hit_tracker.count;

    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_TEST_LED_FLAGS();
        uint8_t amount = 0;

        // Multisplash: every tracked press has its own ring; strongest wins
        for (uint8_t j = 0; j < count; j++) {
            int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
            int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
            uint8_t  dist = sqrt16(dx * dx + dy * dy);
            if (dist > SOLID_SPLASH_MAX_RADIUS) continue;

            uint16_t tick   = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed, 1));
            uint16_t behind = tick - dist;
            uint32_t age    = ((uint32_t)behind * SOLID_FX_LENGTH) / SOLID_SPLASH_RING_WIDTH;
            uint8_t  a      = solid_fx_amount(age);
            if (a > amount) amount = a;
        }

        rgb_t c = amount ? solid_fx_blend(amount, bg_hsv) : bg_rgb;
        rgb_matrix_set_color(i, c.r, c.g, c.b);
    }
    return rgb_matrix_check_finished_leds(led_max);
}
#        endif // ENABLE_RGB_MATRIX_SOLID_MULTISPLASH

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif
