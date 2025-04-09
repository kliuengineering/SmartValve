
<<<<<<< HEAD
// #include "mbed.h"
// #include "RGBLed.h"
// #include "PressureSensor.h"
// #include "ServoMotor.h"

// // UART and I2C Setup
// #define I2C_SDA_PIN PTC11
// #define I2C_SCL_PIN PTC10
// #define I2C_BAUDRATE 50000U
// #define BMP3_I2C_ADDR 0x77

// BufferedSerial uart(PTC4, PTC3, 115200);  // UART to ESP32
// I2C i2c(I2C_SDA_PIN, I2C_SCL_PIN);

// // Devices
// RGBLed led(PTB18, PTB19, PTD0);
// PressureSensor sensor(i2c, BMP3_I2C_ADDR);
// ServoMotor servo(PTC2);
// bool valve_closed = false;

// // Threads and mutex
// Mutex lock_mutex;
// Thread sensorThread;
// Thread commsThread;

// float pressure_hPa = 0.0f;

// void read_sensor_data() {
//     while (true) {
//         float pressure = sensor.readPressure();

//         lock_mutex.lock();
//         pressure_hPa = pressure;
//         printf("Pressure: %.2f hPa\r\n", pressure_hPa);

//         if (pressure_hPa < 950.0f && !valve_closed) {
//             led.setColor(1.0, 0.0, 0.0);  // Red
//             servo = 45.0f;                // Close valve
//             valve_closed = true;
//             printf("Valve CLOSED due to low pressure.\r\n");
//         } else if (pressure_hPa >= 950.0f && valve_closed) {
//             led.setColor(0.0, 1.0, 0.0);  // Green
//             servo = -45.0f;               // Open valve
//             valve_closed = false;
//             printf("Valve OPENED, pressure normal.\r\n");
//         }

//         lock_mutex.unlock();
//         thread_sleep_for(1000);
//     }
// }

// void serve_client() {
//     char buf[64];
//     while (true) {
//         if (uart.readable()) {
//             uart.read(buf, sizeof(buf));
//             if (strncmp(buf, "SENDDATA", 8) == 0) {
//                 lock_mutex.lock();
//                 char response[64];
//                 int len = sprintf(response, "{\"pressure\":%.2f}\r\n", pressure_hPa);
//                 uart.write(response, len);
//                 lock_mutex.unlock();
//             }
//         }
//         thread_sleep_for(100);
//     }
// }

// int main() {
//     printf("FRDM-K66 Booting...\r\n");

//     i2c.frequency(I2C_BAUDRATE);
//     led.setColor(0.0, 0.0, 0.0);  // Off at boot

//     sensor.initialize();

//     commsThread.start(serve_client);
//     sensorThread.start(read_sensor_data);

//     while (true) {
//         ThisThread::sleep_for(1000);
//     }
// }


=======
>>>>>>> 26b790ed9fd903b337b33a7e9605c6972bdb0728
#include "PressureSensor.h"
#include "RGBLed.h"
#include "ServoMotor.h"
#include "mbed.h"
#include <cstdlib>
#include <cstring>

// UART and I2C Setup
#define I2C_SDA_PIN PTC11
#define I2C_SCL_PIN PTC10
#define I2C_BAUDRATE 50000U
#define BMP3_I2C_ADDR 0x77

BufferedSerial uart(PTC4, PTC3, 115200); // UART to ESP32
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

bool parseActuatorCommand(const char *json, int &actuator, int &command) {
  const char *act_key = "\"actuator\"";
  const char *cmd_key = "\"command\"";

  const char *act_ptr = strstr(json, act_key);
  const char *cmd_ptr = strstr(json, cmd_key);
  if (act_ptr == NULL || cmd_ptr == NULL) {
    return false;
  }

  // Find colon after the "actuator" key.
  act_ptr = strchr(act_ptr, ':');
  if (act_ptr == NULL) {
    return false;
  }
  act_ptr++; // move past the colon
  while (*act_ptr == ' ' || *act_ptr == '\t') {
    act_ptr++;
  }
  actuator = atoi(act_ptr);

  // Find colon after the "command" key.
  cmd_ptr = strchr(cmd_ptr, ':');
  if (cmd_ptr == NULL) {
    return false;
  }
  cmd_ptr++; // move past the colon
  while (*cmd_ptr == ' ' || *cmd_ptr == '\t') {
    cmd_ptr++;
  }
  command = atoi(cmd_ptr);

  return true;
}

void read_sensor_data() {
  while (true) {
    float pressure = sensor.readPressure();

    lock_mutex.lock();
    pressure_hPa = pressure;
    printf("Pressure: %.2f hPa\r\n", pressure_hPa);

    // if (pressure_hPa < 950.0f && !valve_closed) {
    //     led.setColor(1.0, 0.0, 0.0);  // Red
    //     servo = 45.0f;                // Close valve
    //     valve_closed = true;
    //     printf("Valve CLOSED due to low pressure.\r\n");
    // } else if (pressure_hPa >= 950.0f && valve_closed) {
    //     led.setColor(0.0, 1.0, 0.0);  // Green
    //     servo = -45.0f;               // Open valve
    //     valve_closed = false;
    //     printf("Valve OPENED, pressure normal.\r\n");
    // }

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
      } else if (strstr(buf, "actuator") != NULL) {
        printf("Actuator command received\n");
        int actuator, command;
        if (parseActuatorCommand(buf, actuator, command)) {
          // For actuator 1 (servo), perform the command:
          if (actuator == 1) {
            if (command == 1 && !valve_closed) {
              led.setColor(1.0, 0.0, 0.0); // Set LED to red.
              servo = 45.0f;               // Move servo to close valve.
              valve_closed = true;
              printf("Valve CLOSED via actuator command.\r\n");
            } else if (command == 0 && valve_closed) {
              led.setColor(0.0, 1.0, 0.0); // Set LED to green.
              servo = -45.0f;              // Move servo to open valve.
              valve_closed = false;
              printf("Valve OPENED via actuator command.\r\n");
            } else {
              printf("Actuator command not applicable for current state.\r\n");
            }
          }
        } else {
          printf("Failed to parse actuator command from JSON.\r\n");
        }
      }
    }
    thread_sleep_for(100);
  }
}

int main() {
  printf("FRDM-K66 Booting...\r\n");

  i2c.frequency(I2C_BAUDRATE);
  led.setColor(0.0, 0.0, 0.0); // Off at boot

  sensor.initialize();

  commsThread.start(serve_client);
  sensorThread.start(read_sensor_data);

  while (true) {
    ThisThread::sleep_for(1000);
  }
<<<<<<< HEAD
}






#include "PressureSensor.h"
#include "RGBLed.h"
#include "ServoMotor.h"
#include "mbed.h"
#include <cstdlib>
#include <cstring>

// UART and I2C Setup
#define I2C_SDA_PIN PTC11
#define I2C_SCL_PIN PTC10
#define I2C_BAUDRATE 50000U
#define BMP3_I2C_ADDR 0x77

BufferedSerial uart(PTC4, PTC3, 115200); // UART to ESP32
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

bool parseActuatorCommand(const char *json, int &actuator, int &command) {
  const char *act_key = "\"actuator\"";
  const char *cmd_key = "\"command\"";

  const char *act_ptr = strstr(json, act_key);
  const char *cmd_ptr = strstr(json, cmd_key);
  if (act_ptr == NULL || cmd_ptr == NULL) {
    return false;
  }

  // Find colon after the "actuator" key.
  act_ptr = strchr(act_ptr, ':');
  if (act_ptr == NULL) {
    return false;
  }
  act_ptr++; // move past the colon
  while (*act_ptr == ' ' || *act_ptr == '\t') {
    act_ptr++;
  }
  actuator = atoi(act_ptr);

  // Find colon after the "command" key.
  cmd_ptr = strchr(cmd_ptr, ':');
  if (cmd_ptr == NULL) {
    return false;
  }
  cmd_ptr++; // move past the colon
  while (*cmd_ptr == ' ' || *cmd_ptr == '\t') {
    cmd_ptr++;
  }
  command = atoi(cmd_ptr);

  return true;
}

void read_sensor_data() {
  while (true) {
    float pressure = sensor.readPressure();

    lock_mutex.lock();
    pressure_hPa = pressure;
    printf("Pressure: %.2f hPa\r\n", pressure_hPa);

    // if (pressure_hPa < 950.0f && !valve_closed) {
    //     led.setColor(1.0, 0.0, 0.0);  // Red
    //     servo = 45.0f;                // Close valve
    //     valve_closed = true;
    //     printf("Valve CLOSED due to low pressure.\r\n");
    // } else if (pressure_hPa >= 950.0f && valve_closed) {
    //     led.setColor(0.0, 1.0, 0.0);  // Green
    //     servo = -45.0f;               // Open valve
    //     valve_closed = false;
    //     printf("Valve OPENED, pressure normal.\r\n");
    // }

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
      } else if (strstr(buf, "actuator") != NULL) {
        printf("Actuator command received\n");
        int actuator, command;
        if (parseActuatorCommand(buf, actuator, command)) {
          // For actuator 1 (servo), perform the command:
          if (actuator == 1) {
            if (command == 1 && !valve_closed) {
              led.setColor(1.0, 0.0, 0.0); // Set LED to red.
              servo = 45.0f;               // Move servo to close valve.
              valve_closed = true;
              printf("Valve CLOSED via actuator command.\r\n");
            } else if (command == 0 && valve_closed) {
              led.setColor(0.0, 1.0, 0.0); // Set LED to green.
              servo = -45.0f;              // Move servo to open valve.
              valve_closed = false;
              printf("Valve OPENED via actuator command.\r\n");
            } else {
              printf("Actuator command not applicable for current state.\r\n");
            }
          }
        } else {
          printf("Failed to parse actuator command from JSON.\r\n");
        }
      }
    }
    thread_sleep_for(100);
  }
}

int main() {
  printf("FRDM-K66 Booting...\r\n");

  i2c.frequency(I2C_BAUDRATE);
  led.setColor(0.0, 0.0, 0.0); // Off at boot

  sensor.initialize();

  commsThread.start(serve_client);
  sensorThread.start(read_sensor_data);

  while (true) {
    ThisThread::sleep_for(1000);
  }
=======
>>>>>>> 26b790ed9fd903b337b33a7e9605c6972bdb0728
}