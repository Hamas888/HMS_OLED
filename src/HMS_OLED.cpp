#include "HMS_OLED.h"
#include <cstring>

#if HMS_OLED_DEBUG_ENABLED
    ChronoLogger *oledLogger = nullptr;
#endif

HMS_OLED::HMS_OLED() : 
    m_buffer(nullptr), 
    m_buffer_size(0), 
    m_driver_type(OLED_DRIVER_TYPE_SSD1306), 
    m_width(HMS_OLED_DEFAULT_WIDTH), 
    m_height(HMS_OLED_DEFAULT_HEIGHT),
    m_i2c_address(HMS_OLED_DEFAULT_ADDRESS)
    #if defined(HMS_OLED_PLATFORM_ARDUINO)
    , m_wire(nullptr)
    #elif defined(HMS_OLED_PLATFORM_ESP_IDF)
    , m_i2c_num(HMS_OLED_DEFAULT_NUM)
    #elif defined(HMS_OLED_PLATFORM_ZEPHYR)
    , m_i2c_dev(nullptr)
    #elif defined(HMS_OLED_PLATFORM_STM32_HAL)
    , m_hi2c(nullptr)
    #endif
{
}

HMS_OLED::~HMS_OLED() {
    freeBuffer();
}

size_t HMS_OLED::calcInternalWidth() const {
    return (m_driver_type == OLED_DRIVER_TYPE_SH1106) ? 132 : 128;
}

#if defined(HMS_OLED_PLATFORM_ARDUINO)
HMS_OLED_StatusTypeDef HMS_OLED::begin(TwoWire *wire, uint8_t address) {
    m_wire = wire;
    m_i2c_address = address;
    m_wire->begin();
    return hwInit();
}
#elif defined(HMS_OLED_PLATFORM_ESP_IDF)
HMS_OLED_StatusTypeDef HMS_OLED::begin(i2c_port_t i2c_port, uint8_t address) {
    m_i2c_num = i2c_port;
    m_i2c_address = address;

    i2c_config_t conf;
    memset(&conf, 0, sizeof(conf));
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = HMS_OLED_DEFAULT_SDA;
    conf.scl_io_num = HMS_OLED_DEFAULT_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = HMS_OLED_DEFAULT_FREQ_HZ;
    esp_err_t r = i2c_param_config(m_i2c_num, &conf);
    if (r != ESP_OK) return HMS_OLED_ERROR;
    r = i2c_driver_install(m_i2c_num, conf.mode, 0, 0, 0);

    if(r == ESP_OK) {
        return hwInit();
    }
    return HMS_OLED_ERROR;
}
#elif defined(HMS_OLED_PLATFORM_ZEPHYR)
HMS_OLED_StatusTypeDef HMS_OLED::begin(const struct device *i2c_dev, uint8_t address) {
    m_i2c_dev = i2c_dev;
    m_i2c_address = address;
    if (!device_is_ready(m_i2c_dev)) return HMS_OLED_ERROR;
    return hwInit();
}
#elif defined(HMS_OLED_PLATFORM_STM32_HAL)
HMS_OLED_StatusTypeDef HMS_OLED::begin(I2C_HandleTypeDef *hi2c, uint8_t address) {
    m_hi2c = hi2c;
    m_i2c_address = address;
    return hwInit();
}
#endif

HMS_OLED_StatusTypeDef HMS_OLED::writeCommand(uint8_t cmd) {
    #if defined(HMS_OLED_PLATFORM_ARDUINO)
        if (!m_wire) return HMS_OLED_ERROR;
        m_wire->beginTransmission(m_i2c_address);
        m_wire->write(0x00); // control byte = command
        m_wire->write(cmd);
        if (m_wire->endTransmission() != 0) return HMS_OLED_ERROR;
        return HMS_OLED_OK;
    #elif defined(HMS_OLED_PLATFORM_ESP_IDF)
        i2c_cmd_handle_t handle = i2c_cmd_link_create();
        i2c_master_start(handle);
        i2c_master_write_byte(handle, (m_i2c_address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(handle, 0x00, true); // control byte = command
        i2c_master_write_byte(handle, cmd, true);
        i2c_master_stop(handle);
        esp_err_t r = i2c_master_cmd_begin(m_i2c_num, handle, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(handle);
        return (r == ESP_OK) ? HMS_OLED_OK : HMS_OLED_ERROR;
    #elif defined(HMS_OLED_PLATFORM_ZEPHYR)
        if (!m_i2c_dev) return HMS_OLED_ERROR;
        uint8_t buf[2] = {0x00, cmd};
        if (i2c_write(m_i2c_dev, buf, 2, m_i2c_address) != 0) return HMS_OLED_ERROR;
        return HMS_OLED_OK;
    #elif defined(HMS_OLED_PLATFORM_STM32_HAL)
        if (!m_hi2c) return HMS_OLED_ERROR;
        uint8_t buf[2] = {0x00, cmd};
        if (HAL_I2C_Master_Transmit(m_hi2c, (uint16_t)(m_i2c_address << 1), buf, 2, 1000) != HAL_OK) return HMS_OLED_ERROR;
        return HMS_OLED_OK;
    #else
        return HMS_OLED_ERROR;
    #endif
}

HMS_OLED_StatusTypeDef HMS_OLED::writeCommands(const uint8_t* cmds, size_t len) {
    HMS_OLED_StatusTypeDef r = HMS_OLED_OK;
    for (size_t i = 0; i < len; i++) {
        r = writeCommand(cmds[i]);
        if (r != HMS_OLED_OK) return r;
    }
    return r;
}

HMS_OLED_StatusTypeDef HMS_OLED::writeData(const uint8_t* data, size_t len) {
    #if defined(HMS_OLED_PLATFORM_ARDUINO)
        if (!m_wire) return HMS_OLED_ERROR;
        m_wire->beginTransmission(m_i2c_address);
        m_wire->write(0x40); // data stream
        m_wire->write(data, len);
        if (m_wire->endTransmission() != 0) return HMS_OLED_ERROR;
        return HMS_OLED_OK;
    #elif defined(HMS_OLED_PLATFORM_ESP_IDF)
        i2c_cmd_handle_t handle = i2c_cmd_link_create();
        i2c_master_start(handle);
        i2c_master_write_byte(handle, (m_i2c_address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(handle, 0x40, true); // data stream
        i2c_master_write(handle, (uint8_t*)data, len, true);
        i2c_master_stop(handle);
        esp_err_t r = i2c_master_cmd_begin(m_i2c_num, handle, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(handle);
        return (r == ESP_OK) ? HMS_OLED_OK : HMS_OLED_ERROR;
    #elif defined(HMS_OLED_PLATFORM_ZEPHYR)
        if (!m_i2c_dev) return HMS_OLED_ERROR;
        uint8_t buf[len + 1];
        buf[0] = 0x40;
        memcpy(&buf[1], data, len);
        if (i2c_write(m_i2c_dev, buf, len + 1, m_i2c_address) != 0) return HMS_OLED_ERROR;
        return HMS_OLED_OK;
    #elif defined(HMS_OLED_PLATFORM_STM32_HAL)
        if (!m_hi2c) return HMS_OLED_ERROR;
        uint8_t buf[len + 1];
        buf[0] = 0x40;
        memcpy(&buf[1], data, len);
        if (HAL_I2C_Master_Transmit(m_hi2c, (uint16_t)(m_i2c_address << 1), buf, len + 1, 1000) != HAL_OK) return HMS_OLED_ERROR;
        return HMS_OLED_OK;
    #else
        return HMS_OLED_ERROR;
    #endif
}

bool HMS_OLED::detectSH1106() {
    HMS_OLED_StatusTypeDef r = writeCommand(0xB0 + 7);
    return (r == HMS_OLED_OK);
}

void HMS_OLED::detectDriver(void) {
    if (detectSH1106()) {
        HMS_OLED_LOGGER(info, "Detected SH1106");
        m_driver_type = OLED_DRIVER_TYPE_SH1106;
    } else {
        HMS_OLED_LOGGER(info, "Assuming SSD1306");
        m_driver_type = OLED_DRIVER_TYPE_SSD1306;
    }
}

HMS_OLED_StatusTypeDef HMS_OLED::allocateBuffer(void) {
    freeBuffer();

    size_t internal_w = calcInternalWidth();
    m_buffer_size = (internal_w * m_height) / 8;
    m_buffer = (uint8_t*) malloc(m_buffer_size);
    if (!m_buffer) {
        HMS_OLED_LOGGER(error, "Failed to allocate oled buffer (%d bytes)", (int)m_buffer_size);
        m_buffer_size = 0;
        return HMS_OLED_NO_MEM;
    }
    memset(m_buffer, 0, m_buffer_size);
    HMS_OLED_LOGGER(info, "OLED buffer allocated %d bytes (internal width=%d height=%d)",
             (int)m_buffer_size, (int)internal_w, (int)m_height);
    return HMS_OLED_OK;
}

void HMS_OLED::freeBuffer(void) {
    if (m_buffer) {
        free(m_buffer);
        m_buffer = nullptr;
        m_buffer_size = 0;
    }
}

HMS_OLED_StatusTypeDef HMS_OLED::hwInit(void) {
    const uint8_t init_seq[] = {
        0xAE,       // display off
        0xD5, 0x80, // set display clock divide ratio/oscillator frequency
        0xA8, 0x3F, // set multiplex ratio(1 to 64) -> 0x3F for 64
        0xD3, 0x00, // set display offset
        0x40,       // set start line = 0
        0x8D, 0x14, // enable charge pump
        0x20, 0x02, // memory addressing mode = page addressing mode
        0xA1,       // segment remap (column address 127 is mapped to SEG0)
        0xC8,       // COM output scan direction remapped
        0xDA, 0x12, // COM pins hardware configuration
        0x81, 0xCF, // contrast
        0xD9, 0xF1, // pre-charge
        0xDB, 0x40, // VCOMH deselect level
        0xA4,       // resume to RAM content
        0xA6,       // normal display (not inverted)
        0x2E,       // deactivate scroll
        0xAF        // display ON
    };
    return writeCommands(init_seq, sizeof(init_seq));
}

void HMS_OLED::deinit(void) {
    writeCommand(0xAE);
    freeBuffer();
}

void HMS_OLED::clear(void) {
    if (m_buffer && m_buffer_size)
        memset(m_buffer, 0, m_buffer_size);
}

void HMS_OLED::fill(uint8_t pattern) {
    if (m_buffer && m_buffer_size)
        memset(m_buffer, pattern, m_buffer_size);
}

void HMS_OLED::setPixel(int x, int y, bool color) {
    if (!m_buffer) return;
    if (x < 0 || x >= m_width) return;
    if (y < 0 || y >= m_height) return;

    size_t internal_w = calcInternalWidth();
    size_t byte_index = x + (y / 8) * internal_w;

    if (byte_index >= m_buffer_size) return;

    if (color)
        m_buffer[byte_index] |= (1 << (y & 7));
    else
        m_buffer[byte_index] &= ~(1 << (y & 7));
}

void HMS_OLED::drawChar(int x, int y, char c) {
    if (c < 0x20 || c > 0x7E) c = '?';
    const uint8_t* glyph = font_5x7[c - 0x20];

    for (int col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (int row = 0; row < 8; row++) {
            setPixel(x + col, y + row, (line >> row) & 1);
        }
    }
}

void HMS_OLED::drawText(int x, int y, const char* text) {
    while (*text) {
        drawChar(x, y, *text);
        x += 6; // 5px + 1 spacing
        text++;
    }
}

void HMS_OLED::drawInt(int x, int y, int value){
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    drawText(x, y, buf);
}

void HMS_OLED::drawFloat(int x, int y, float value, int decimals) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", decimals, value);
    drawText(x, y, buf);
}

void HMS_OLED::clearRect(int x, int y, int width, int height) {
    for (int i = x; i < x + width; i++) {
        for (int j = y; j < y + height; j++) {
            setPixel(i, j, false);
        }
    }
}

void HMS_OLED::drawLine(int x0, int y0, int x1, int y1, bool color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        setPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void HMS_OLED::drawRect(int x, int y, int width, int height, bool color) {
    drawLine(x, y, x + width - 1, y, color);
    drawLine(x, y + height - 1, x + width - 1, y + height - 1, color);
    drawLine(x, y, x, y + height - 1, color);
    drawLine(x + width - 1, y, x + width - 1, y + height - 1, color);
}

void HMS_OLED::drawBitmap(int x, int y, const uint8_t* bitmap, int w, int h) {
    if (!bitmap) return;
    int bytes_per_row = (w + 7) / 8;
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            if (bitmap[j * bytes_per_row + (i / 8)] & (0x80 >> (i % 8))) {
                setPixel(x + i, y + j, true);
            } else {
                setPixel(x + i, y + j, false);
            }
        }
    }
}

HMS_OLED_StatusTypeDef HMS_OLED::display(void) {
    if (!m_buffer) return HMS_OLED_ERROR;

    HMS_OLED_StatusTypeDef r = HMS_OLED_OK;
    size_t internal_w = calcInternalWidth();
    int pages = m_height / 8;

    for (int p = 0; p < pages; p++) {
        uint8_t cmds[] = {
            (uint8_t)(0xB0 + p),      // page addr
            0x00,                     // lower col start
            0x10                      // higher col start
        };
        r = writeCommands(cmds, sizeof(cmds));
        if (r != HMS_OLED_OK) return r;

        r = writeData(&m_buffer[p * internal_w], internal_w);
        if (r != HMS_OLED_OK) return r;
    }
    return r;
}
