#ifndef __PRESSURESENSOR_H__
#define __PRESSURESENSOR_H__

#include "mbed.h"
#include "fsl_i2c.h"
#include "bmp3.h"

#define I2C_BAUDRATE 50000U

class PressureSensor {
private:
    I2C& i2c;
    bmp3_dev bmp;
    bmp3_data data;
    bmp3_settings settings;
    float pressure;
    uint8_t address;

    static int8_t i2cRead(uint8_t reg_addr, uint8_t* reg_data, uint32_t len, void* intf_ptr) {
        auto* self = static_cast<PressureSensor*>(intf_ptr);
        char reg = reg_addr;
        if (self->i2c.write(self->address << 1, &reg, 1, true) != 0) return -1;
        return self->i2c.read(self->address << 1, (char*)reg_data, len);
    }

    static int8_t i2cWrite(uint8_t reg_addr, const uint8_t* reg_data, uint32_t len, void* intf_ptr) {
        auto* self = static_cast<PressureSensor*>(intf_ptr);
        char buffer[len + 1];
        buffer[0] = reg_addr;
        memcpy(&buffer[1], reg_data, len);
        return self->i2c.write(self->address << 1, buffer, len + 1);
    }

    static void delayUs(uint32_t us, void* /* intf_ptr */) {
        wait_us(us);
    }

public:
    PressureSensor(I2C& i2c_interface, uint8_t addr = 0x77)
        : i2c(i2c_interface), address(addr), pressure(0.0f) {
        i2c.frequency(I2C_BAUDRATE);
        bmp.intf = BMP3_I2C_INTF;
        bmp.read = i2cRead;
        bmp.write = i2cWrite;
        bmp.delay_us = delayUs;
        bmp.intf_ptr = this;  // Use 'this' to access members in static functions
    }

    void init() {
        int8_t result = bmp3_init(&bmp);
        if (result != BMP3_OK) {
            printf("BMP384 init failed: %d\r\n", result);
            while (1);
        }

        settings.op_mode = BMP3_MODE_NORMAL;
        settings.press_en = BMP3_ENABLE;
        settings.temp_en = BMP3_DISABLE;
        settings.odr_filter.press_os = BMP3_OVERSAMPLING_16X;
        settings.odr_filter.iir_filter = BMP3_IIR_FILTER_COEFF_3;

        uint16_t settings_sel = BMP3_SEL_PRESS_EN | BMP3_SEL_PRESS_OS | BMP3_SEL_IIR_FILTER;
        bmp3_set_sensor_settings(settings_sel, &settings, &bmp);
        bmp3_set_op_mode(&settings, &bmp);
    }

    // Alias to match your previous call
    void initialize() {
        init();
    }

    float readPressure() {
        if (bmp3_get_sensor_data(BMP3_PRESS, &data, &bmp) == BMP3_OK) {
            pressure = data.pressure / 100.0f;
        }
        return pressure;
    }
};

#endif
