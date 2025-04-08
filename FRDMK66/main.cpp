
#include "mbed.h"
#include "RGBLed.h"
#include "PressureSensor.h"
#include "ServoMotor.h"

// UART and I2C Setup
#define I2C_SDA_PIN PTC11
#define I2C_SCL_PIN PTC10
#define I2C_BAUDRATE 50000U
#define BMP3_I2C_ADDR 0x77

BufferedSerial uart(PTC4, PTC3, 115200);  // UART to ESP32
I2C i2c(I2C_SDA_PIN, I2C_SCL_PIN);

// Devices
RGBLed led(PTB18, PTB19, PTD0);
PressureSensor sensor(i2c, BMP3_I2C_ADDR);
ServoMotor servo(PTC2);
bool valve_closed = false;

// Threads and mutex
Mutex lock_mutex;
Thread sensorThread;
Thread commsThread;

float pressure_hPa = 0.0f;

void read_sensor_data() {
    while (true) {
        float pressure = sensor.readPressure();

        lock_mutex.lock();
        pressure_hPa = pressure;
        printf("Pressure: %.2f hPa\r\n", pressure_hPa);

        if (pressure_hPa < 950.0f && !valve_closed) {
            led.setColor(1.0, 0.0, 0.0);  // Red
            servo = 45.0f;                // Close valve
            valve_closed = true;
            printf("Valve CLOSED due to low pressure.\r\n");
        } else if (pressure_hPa >= 950.0f && valve_closed) {
            led.setColor(0.0, 1.0, 0.0);  // Green
            servo = -45.0f;               // Open valve
            valve_closed = false;
            printf("Valve OPENED, pressure normal.\r\n");
        }

        lock_mutex.unlock();
        thread_sleep_for(1000);
    }
}

void serve_client() {
    char buf[64];
    while (true) {
        if (uart.readable()) {
            uart.read(buf, sizeof(buf));
            if (strncmp(buf, "SENDDATA", 8) == 0) {
                lock_mutex.lock();
                char response[64];
                int len = sprintf(response, "{\"pressure\":%.2f}\r\n", pressure_hPa);
                uart.write(response, len);
                lock_mutex.unlock();
            }
        }
        thread_sleep_for(100);
    }
}

int main() {
    printf("FRDM-K66 Booting...\r\n");

    i2c.frequency(I2C_BAUDRATE);
    led.setColor(0.0, 0.0, 0.0);  // Off at boot

    sensor.initialize();

    commsThread.start(serve_client);
    sensorThread.start(read_sensor_data);

    while (true) {
        ThisThread::sleep_for(1000);
    }
}
