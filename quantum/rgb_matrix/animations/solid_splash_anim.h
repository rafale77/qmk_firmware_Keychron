#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
#    if defined(ENABLE_RGB_MATRIX_SOLID_SPLASH) || defined(ENABLE_RGB_MATRIX_SOLID_MULTISPLASH)

#        ifdef ENABLE_RGB_MATRIX_SOLID_SPLASH
RGB_MATRIX_EFFECT(SOLID_SPLASH)

// Fixed background color; override in the keyboard's config.h
#        ifndef SOLID_SPLASH_BG_R
#            define SOLID_SPLASH_BG_R 0
#        endif
#        ifndef SOLID_SPLASH_BG_G
#            define SOLID_SPLASH_BG_G 255
#        endif
#        ifndef SOLID_SPLASH_BG_B
#            define SOLID_SPLASH_BG_B 255
#        endif



#    endif // ENABLE_RGB_MATRIX_SOLID_SPLASH
//#        endif

#        ifdef ENABLE_RGB_MATRIX_SOLID_MULTISPLASH
RGB_MATRIX_EFFECT(SOLID_MULTISPLASH)
#        endif

#        ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

hsv_t SOLID_SPLASH_math(hsv_t hsv, int16_t dx, int16_t dy, uint8_t dist, uint16_t tick) {
    uint16_t effect = tick - dist;
    if (effect > 255) effect = 255;
    hsv.v = qadd8(hsv.v, 255 - effect);
    return hsv;
}

 #            ifdef ENABLE_RGB_MATRIX_SOLID_SPLASH

 bool SOLID_SPLASH(effect_params_t* params) {
     RGB_MATRIX_USE_LIMITS(led_min, led_max);

     uint8_t count = g_last_hit_tracker.count;
     RGB     sp    = rgb_matrix_hsv_to_rgb(rgb_matrix_config.hsv); // splash = user HSV

     for (uint8_t i = led_min; i < led_max; i++) {
         RGB_MATRIX_TEST_LED_FLAGS();

         uint8_t amount = 0; // 0 = background, 255 = full splash

         for (uint8_t j = 0; j < count; j++) {
             int16_t  dx   = g_led_config.point[i].x - g_last_hit_tracker.x[j];
             int16_t  dy   = g_led_config.point[i].y - g_last_hit_tracker.y[j];
             uint8_t  dist = sqrt16(dx * dx + dy * dy);
             uint16_t tick = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed, 1));
             uint16_t eff  = tick - dist;
             if (eff > 255) eff = 255;
             amount = qadd8(amount, 255 - eff);
         }

         uint8_t inv = 255 - amount;
         rgb_matrix_set_color(i,
             ((uint16_t)SOLID_SPLASH_BG_R * inv + (uint16_t)sp.r * amount) / 255,
             ((uint16_t)SOLID_SPLASH_BG_G * inv + (uint16_t)sp.g * amount) / 255,
             ((uint16_t)SOLID_SPLASH_BG_B * inv + (uint16_t)sp.b * amount) / 255);
     }

     return rgb_matrix_check_finished_leds(led_max);
 }
 #    endif // ENABLE_RGB_MATRIX_SOLID_SPLASH
// bool SOLID_SPLASH(effect_params_t* params) {
//    return effect_runner_reactive_splash(qsub8(g_last_hit_tracker.count, 1), params, &SOLID_SPLASH_math);
//}
//#            endif

#            ifdef ENABLE_RGB_MATRIX_SOLID_MULTISPLASH
bool SOLID_MULTISPLASH(effect_params_t* params) {
    return effect_runner_reactive_splash(0, params, &SOLID_SPLASH_math);
}
#            endif

#        endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#    endif     // !defined(ENABLE_RGB_MATRIX_SPLASH) && !defined(ENABLE_RGB_MATRIX_MULTISPLASH)
#endif         // RGB_MATRIX_KEYREACTIVE_ENABLED
