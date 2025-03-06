#include "mbed.h"
#include "platform/mbed_thread.h"
#include <cstdlib>
#include <cstring>
#include <string.h>
#include <string>

// Blinking rate in milliseconds
#define BLINKING_RATE_MS 500ms

BufferedSerial uart1_port(PTC4, PTC3, 115200);

int sensor = 1;
int sensor_val = 1;

Mutex lock_mutex;
Thread serve_client_thread;
Thread read_sensor_data_thread;

void serve_client();
void read_sensor_data();

int main() {
  // Initialize LED once
  DigitalOut led(LED1);

  printf("Please press Enter to continue...\n");
  char x = getchar();

  serve_client_thread.start(callback(serve_client));
  read_sensor_data_thread.start(callback(read_sensor_data));

  while (true) {
    // Sleep for the defined blinking rate
    ThisThread::sleep_for(BLINKING_RATE_MS);
    // Toggle the LED and print message when LED is ON
    led = !led;
  }
}

// One thread to talk with client via ESP32
void serve_client() {
  char buf[128]; // Buffer for UART data
  size_t bytesRead;

  while (true) {
    if (uart1_port.readable()) {
      // Read up to (sizeof(buf)-1) bytes from UART1
      bytesRead = uart1_port.read(buf, sizeof(buf) - 1);
      if (bytesRead > 0) {
        buf[bytesRead] = '\0';
        string buffer = buf;

        if (buffer.find("SENDDATA") != std::string::npos) {
          printf("K66F - SENDDATA command received\n");
          char jsonMsg[64];
          lock_mutex.lock();
          int size = sprintf(jsonMsg, "{\"sensor\": %d, \"value\": %d}", sensor,
                             sensor_val);
          uart1_port.write(jsonMsg, size);
          printf("K66F - send %s\n", jsonMsg);
          lock_mutex.unlock();
        } else if (buffer.find("actuator") != std::string::npos) {
          printf("actuator command received\n");
          // TODO: Perform the command on the actuator
          // ...
        }
        printf("K66F - Received %s\n\n\r", buf);
      }
    }
    ThisThread::sleep_for(500ms);
  }
}

// One thread to read data from the sensor
// .... Fake sensor data for now
void read_sensor_data() {
  // TODO: Read data from the sensor
  // ..
  while (true) {
    lock_mutex.lock();
    sensor = 0;
    sensor_val = std::rand();
    lock_mutex.unlock();
    ThisThread::sleep_for(500ms);
  }
}