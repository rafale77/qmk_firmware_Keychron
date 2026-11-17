// Copyright 2023 luantty2 (@luantty2)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef RGB_MATRIX_ENABLE
// IS31FL3733 I2C addresses (ADDR1/ADDR2 strapping): 0x50 and 0x5C
#    define IS31FL3733_I2C_ADDRESS_1 IS31FL3733_I2C_ADDRESS_GND_GND
#    define IS31FL3733_I2C_ADDRESS_2 IS31FL3733_I2C_ADDRESS_VCC_GND

// Driver 0 generates the sync clock, driver 1 follows it
#    define IS31FL3733_SYNC_1 IS31FL3733_SYNC_MASTER
#    define IS31FL3733_SYNC_2 IS31FL3733_SYNC_SLAVE

// Persistent storage for the Solid FX settings (4 bytes: hue, sat, val, fade)
#    define EECONFIG_KB_DATA_SIZE 5

#    define IS31FL3733_GLOBAL_CURRENT 0xC0   // default is 0xFF
#    define IS31FL3733_SW_PULLUP   IS31FL3733_PUR_8K_OHM
#    define IS31FL3733_CS_PULLDOWN IS31FL3733_PDR_8K_OHM
#endif

// Correct colors for strange strong blue on Aaru. Orange
#define SOLID_FX_HUE_DEG 24    // animation color hue, 0-360 (60 = yellow) 34 before white balance 23 after
#define SOLID_FX_SAT_PCT 100   // 0-100
#define SOLID_FX_VAL_PCT 100   // 0-100

#define SOLID_FX_FADE_PCT 50   // share of the animation spent fading out
// #define SOLID_FX_LENGTH 255 // optional: animation lifetime in ticks (stock = 255)

// Global white balance: 255 = unchanged, lower = weaker channel. Tune by eye.
#    define AARU87_GAIN_R 140
#    define AARU87_GAIN_G 255
#    define AARU87_GAIN_B 120
#    define AARU87_BOARD_B   // uncomment when flashing your second PCB
