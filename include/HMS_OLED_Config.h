/*
 ============================================================================================================================================
 * File:        HMS_OLED_Config.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Dec 24 2025
 * Brief:       This file package provides OLED display driver configuration options.
 ============================================================================================================================================
 * License: 
 * MIT License
 * 
 * Copyright (c) 2025 Hamas Saeed
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do 
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For any inquiries, contact Hamas Saeed at hamasaeed@gmail.com
 ============================================================================================================================================
 */

#ifndef HMS_OLED_CONFIG_H
#define HMS_OLED_CONFIG_H

/*
  ┌─────────────────────────────────────────────────────────────────────┐
  │ Note:     Platform detection                                        │
  │ Requires: Development environment and platform-specific SDKs        │
  └─────────────────────────────────────────────────────────────────────┘
*/
#if defined(ARDUINO)
    #include <Arduino.h>
    #if defined(ESP32)
        #define HMS_OLED_ARDUINO_ESP32
    #elif defined(ESP8266)
        // Include ESP8266 MQTT libraries here
        #define HMS_OLED_ARDUINO_ESP8266
    #endif
    #define HMS_OLED_PLATFORM_ARDUINO
#elif defined(ESP_PLATFORM)
    #include <cstdio>
    #include <stdio.h>
    #include <cstring>
    #include <cstdlib>
    #include <stdint.h>
    #include "driver/i2c.h"
    #include "esp_err.h"
    #include "esp_log.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #define HMS_OLED_PLATFORM_ESP_IDF
#elif defined(__ZEPHYR__)
    #include <zephyr/kernel.h>
    #include <zephyr/device.h>
    #include <zephyr/drivers/i2c.h>
    #define HMS_OLED_PLATFORM_ZEPHYR
#elif defined(__STM32__) || defined(STM32F0) || defined(STM32F1) || defined(STM32F3) || defined(STM32F4) || \
      defined(STM32F7) || defined(STM32G0) || defined(STM32G4) || defined(STM32H7) || \
      defined(STM32L0) || defined(STM32L1) || defined(STM32L4) || defined(STM32L5) || \
      defined(STM32WB) || defined(STM32WL)
    #include "stm32_hal_legacy.h" // Or specific HAL header if known
    // Usually users include their specific hal header before this
    #define HMS_OLED_PLATFORM_STM32_HAL
#elif defined(__linux__) || defined(_WIN32) || defined(__APPLE__)
    // Desktop specific includes
    #define HMS_OLED_PLATFORM_DESKTOP
#endif // Platform detection

/*
  ┌─────────────────────────────────────────────────────────────────────┐
  │ Note:     Enable only if ChronoLog is included                      │
  │ Requires: ChronoLog library → https://github.com/Hamas888/ChronoLog │
  └─────────────────────────────────────────────────────────────────────┘
*/
#if defined(CONFIG_HMS_OLED_DEBUG)
    #define HMS_OLED_DEBUG_ENABLED              1
#elif defined(HMS_OLED_DEBUG)
    #define HMS_OLED_DEBUG_ENABLED              1
#else
    #define HMS_OLED_DEBUG_ENABLED              0                            // Enable debug messages (1=enabled, 0=disabled)
#endif


#if HMS_OLED_DEBUG_ENABLED
  #if __has_include("ChronoLog.h")
    #include "ChronoLog.h"
    #ifndef HMS_OLED_LOG_LEVEL
      #define HMS_OLED_LOG_LEVEL                CHRONOLOG_LEVEL_DEBUG
      extern ChronoLogger *oledLogger;
    #endif
    #define HMS_OLED_LOGGER(level, msg, ...)      \
      do {                                        \
        if (oledLogger)                           \
          oledLogger->level(                      \
            msg, ##__VA_ARGS__                    \
          );                                      \
      } while (0)
  #else
    #define HMS_OLED_LOGGER(level, msg, ...) ESP_LOGI("HMS_OLED", msg, ##__VA_ARGS__)
  #endif
#else
  #define HMS_OLED_LOGGER(level, msg, ...) do {} while (0)
#endif

/*
  ┌─────────────────────────────────────────────────────────────────────┐
  │ Note: WAZEH CycleTime Custom Types & Definitions                    │
  └─────────────────────────────────────────────────────────────────────┘
*/

#define HMS_OLED_DEFAULT_ADDRESS                0x3C

#define HMS_OLED_DEFAULT_WIDTH                  128
#define HMS_OLED_DEFAULT_HEIGHT                 64

#define HMS_OLED_DEFAULT_SCL                    12
#define HMS_OLED_DEFAULT_SDA                    13
#if defined(HMS_OLED_PLATFORM_ESP_IDF)
    #define HMS_OLED_DEFAULT_NUM                I2C_NUM_0
#else
    #define HMS_OLED_DEFAULT_NUM                0
#endif
#define HMS_OLED_DEFAULT_FREQ_HZ                400000

#define HMS_OLED_ALL_FONTS
#define HMS_OLED_SMALL_FONT

typedef enum {
    HMS_OLED_OK       = 0x00,
    HMS_OLED_ERROR    = 0x01,
    HMS_OLED_BUSY     = 0x02,
    HMS_OLED_TIMEOUT  = 0x03,
    HMS_OLED_NOT_FOUND= 0x04,
    HMS_OLED_NO_MEM   = 0x05
} HMS_OLED_StatusTypeDef;

typedef enum {
    OLED_DRIVER_TYPE_SH1106  = 1,
    OLED_DRIVER_TYPE_SSD1306 = 0
} HMS_OLED_DriverType;

#include "HMS_OLED_Fonts.h"

#endif // HMS_OLED_CONFIG_H