
#include "mbed.h"
#include "RGBLed.h"
#include "PressureSensor.h"
#include "ServoMotor.h"

using namespace std::chrono;

// --- Config ---
#define I2C_SDA_PIN PTC11
#define I2C_SCL_PIN PTC10
#define I2C_BAUDRATE 50000U
#define BMP3_I2C_ADDR 0x77
#define UART_TX PTC4
#define UART_RX PTC3
#define UART_BAUD 115200

// --- Interfaces ---
BufferedSerial uart(UART_TX, UART_RX, UART_BAUD);
I2C i2c(I2C_SDA_PIN, I2C_SCL_PIN);

// --- Devices ---
RGBLed led(PTB18, PTB19, PTD0);
PressureSensor sensor(i2c, BMP3_I2C_ADDR);
ServoMotor servo(PTC2);
bool valve_closed = false;

// --- Threads ---
Mutex lock_mutex;
Thread sensorThread;
Thread commsThread;

float pressure_hPa = 0.0f;

// --- Parse JSON command from ESP32 ---
bool parseActuatorCommand(const char* json, int& actuator, int& command) {
    const char* act_key = "\"actuator\"";
    const char* cmd_key = "\"command\"";
    const char* act_ptr = strstr(json, act_key);
    const char* cmd_ptr = strstr(json, cmd_key);
    if (!act_ptr || !cmd_ptr) return false;

    act_ptr = strchr(act_ptr, ':');
    cmd_ptr = strchr(cmd_ptr, ':');
    if (!act_ptr || !cmd_ptr) return false;

    actuator = atoi(++act_ptr);
    command = atoi(++cmd_ptr);

    return true;
}

// --- Read pressure sensor periodically ---
void read_sensor_data() {
    while (true) {
        float pressure = sensor.readPressure();
        lock_mutex.lock();
        pressure_hPa = pressure;
        printf("Pressure: %.2f hPa\n", pressure_hPa);
        lock_mutex.unlock();
        ThisThread::sleep_for(1s);
    }
}

// --- UART Command Handler (from ESP32) ---
void serve_client() {
    char buf[64];
    while (true) {
        if (uart.readable()) {
            uart.read(buf, sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';

            if (strncmp(buf, "SENDDATA", 8) == 0) {
                lock_mutex.lock();
                char response[64];
                int len = sprintf(response, "{\"pressure\":%.2f}", pressure_hPa);
                uart.write(response, len);
                printf("Sent to ESP32: %s\n", response);
                lock_mutex.unlock();
            } 
            else if (strstr(buf, "actuator")) {
                int actuator, command;
                if (parseActuatorCommand(buf, actuator, command)) {
                    if (actuator == 1) {
                        if (command == 1 && !valve_closed) {
                            led.setColor(1.0, 0.0, 0.0);  // Red = Close
                            servo = 45.0f;
                            valve_closed = true;
                            printf("Valve CLOSED by AI command.\n");
                        } else if (command == 0 && valve_closed) {
                            led.setColor(0.0, 1.0, 0.0);  // Green = Open
                            servo = -45.0f;
                            valve_closed = false;
                            printf("Valve OPENED by AI command.\n");
                        }
                    }
                } else {
                    printf("Failed to parse actuator command.\n");
                }
            }
        }
        ThisThread::sleep_for(100ms);
    }
}

// --- Entry Point ---
int main() {
    printf("FRDM-K66 Booting...\n");

    i2c.frequency(I2C_BAUDRATE);
    led.setColor(0.0, 0.0, 0.0);  // LED OFF on boot
    sensor.initialize();

    sensorThread.start(read_sensor_data);
    commsThread.start(serve_client);

    while (true) {
        ThisThread::sleep_for(1s);
    }
}

