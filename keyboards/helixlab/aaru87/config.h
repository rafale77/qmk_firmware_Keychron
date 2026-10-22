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
#endif

// Correct colors for strange strong blue on Aaru. Cyan
# define FIXED_BG_H 145
# define FIXED_BG_S 100
# define FIXED_BG_V 100
