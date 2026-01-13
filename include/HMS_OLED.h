/*
 ============================================================================================================================================
 * File:        HMS_OLED.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Dec 24 2025
 * Brief:       This file package provides OLED display driver for embedded systems (Arduino, ESP-IDF, Zephyr, STM32 HAL).
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

#ifndef HMS_OLED_H
#define HMS_OLED_H

#include "HMS_OLED_Config.h"

class HMS_OLED {
public:
    HMS_OLED();
    ~HMS_OLED();

    #if defined(HMS_OLED_PLATFORM_ARDUINO)
        HMS_OLED_StatusTypeDef begin(TwoWire *wire = &Wire, uint8_t address = HMS_OLED_DEFAULT_ADDRESS);
    #elif defined(HMS_OLED_PLATFORM_ESP_IDF)
        HMS_OLED_StatusTypeDef begin(i2c_port_t i2c_port = I2C_NUM_0, uint8_t address = HMS_OLED_DEFAULT_ADDRESS);
    #elif defined(HMS_OLED_PLATFORM_ZEPHYR)
        HMS_OLED_StatusTypeDef begin(const struct device *i2c_dev, uint8_t address = HMS_OLED_DEFAULT_ADDRESS);
    #elif defined(HMS_OLED_PLATFORM_STM32_HAL)
        HMS_OLED_StatusTypeDef begin(I2C_HandleTypeDef *hi2c, uint8_t address = HMS_OLED_DEFAULT_ADDRESS);
    #endif

    void detectDriver(void);

    HMS_OLED_StatusTypeDef allocateBuffer(void);

    void freeBuffer(void);

    HMS_OLED_StatusTypeDef hwInit(void);
    void deinit(void);
    HMS_OLED_StatusTypeDef display(void);
    void clear(void);

    void fill(uint8_t pattern);

    void setPixel(int x, int y, bool color);

    void drawChar(int x, int y, char c);
    void drawText(int x, int y, const char* text);

    void drawInt(int x, int y, int value);

    void drawFloat(int x, int y, float value, int decimals);
    void clearRect(int x, int y, int width, int height);
    void drawLine(int x0, int y0, int x1, int y1, bool color);
    void drawRect(int x, int y, int width, int height, bool color);
    void drawBitmap(int x, int y, const uint8_t* bitmap, int w, int h);

    uint8_t getDriverType() const { return m_driver_type; }
    uint16_t getWidth() const { return m_width; }
    uint16_t getHeight() const { return m_height; }

private:
    HMS_OLED_StatusTypeDef writeCommand(uint8_t cmd);
    HMS_OLED_StatusTypeDef writeCommands(const uint8_t* cmds, size_t len);
    HMS_OLED_StatusTypeDef writeData(const uint8_t* data, size_t len);
    bool detectSH1106();
    size_t calcInternalWidth() const;

    uint8_t* m_buffer;
    size_t m_buffer_size;
    uint8_t m_driver_type;
    uint16_t m_width;
    uint16_t m_height;
    uint8_t m_i2c_address;

    #if defined(HMS_OLED_PLATFORM_ARDUINO)
    TwoWire *m_wire;
    #elif defined(HMS_OLED_PLATFORM_ESP_IDF)
    i2c_port_t m_i2c_num;
    #elif defined(HMS_OLED_PLATFORM_ZEPHYR)
    const struct device *m_i2c_dev;
    #elif defined(HMS_OLED_PLATFORM_STM32_HAL)
    I2C_HandleTypeDef *m_hi2c;
    #endif
};

#endif // HMS_OLED_H