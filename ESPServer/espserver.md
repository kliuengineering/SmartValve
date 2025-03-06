## ESP32 server

It acts like a bridge between clients and the K66F
## Hardware setup

#### Set up ESP32
1. Download and install Arduino IDE
2. Go to `Board Manager` -> install `esp32 by Espressif Systems`
3. Upload `espserver.ino` under ESPServer folder into the ESP32

#### Set up K66F
1. Open `Keil IDE`
2. Create a new project with an empty mbedOS project.
3. Copy `frdmk66.example.cpp` into `main.cpp`
4. Build the project into binary file
5. Drag and drop the binary file into the K66F

#### Set up hardware for both ESP32 AND K66F
<img src="https://github.com/kliuengineering/SmartValve/master/pics/ESPServer_hardware_setup.png"/>


## API endpoints

#### GET: /ip-address/

Inital request to be sure the ESP32 server is working.

```text
an html page will be displayed
```

#### GET: /ip-address/sensor

Client requests sensor values from K66F via the ESP32 server.

```json
{
  "sensor": 1,
  "value": 12
}
```

#### POST: /ip-address/actuator

Client sends command to actuators attached to K66F via the ESP32 server.

```json
{
  "actuator": 1,
  "command": 0
}
```

## Expected results

<img src="https://github.com/kliuengineering/SmartValve/master/pics/ESPServer_result_1.png"/>

<img src="https://github.com/kliuengineering/SmartValve/master/pics/ESPServer_result_2.png"/>

<img src="https://github.com/kliuengineering/SmartValve/master/pics/ESPServer_result_3.png"/>