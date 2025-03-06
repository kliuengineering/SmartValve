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

#### Set up hardware for both ESP32 and K66F (hardware)

<img src="https://github.com/kliuengineering/SmartValve/blob/master/pics/ESPServer_hardware_setup.png"/>

#### Set up hardware for both ESP32 and K66F (software)

1. Connect K66F with your computer using USB cable
2. run

```bash
ls /dev/cu.* # or do whatever to find the port for K66F on your computer
screen /dev/cu.tty-14031 # dude, this port is mine, so put yours in here

Please press Enter to continue... # this is what we should see, so press Enter
```

3. This step is important, press EN button on ESP32 (maybe the RESET button). At the same time, we also have to turn `OFF` and then `ON` the Personal Hotspot on your phone. If you don't see it connecting with your Hotspot, wait for 1 second and repeat, and keep doing so until you see:

```bash
Please press Enter to continue...
K66F - Received ets Jul 29 2019 12:21:46

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_d

K66F - Received rv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4916
load:0x40078000,len:1

K66F - Received 64�

ESP - Connecting to tovn


K66F - Received ESP - WiFi Connected! IP Address: 172.20.10.3 # If you don't see this, keep doing step 3 until you see the IP Address. *Note: repeat after 1 or 2 seconds.
ESP - HTTP server started on port 80
```

## API endpoints

#### GET: /ip-address/

Inital request to be sure the ESP32 server is working.

```html
<!DOCTYPE html> <html>
<head><meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>Rice</title>
<h1>Chicken Wings with Fried Rice</h1>
<h3>Using WIFI Mode</h3>
<p>Device IP Address: <strong>172.20.10.3</strong></p></body>
</html>
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

<img src="https://github.com/kliuengineering/SmartValve/blob/master/pics/ESPServer_result_1.png"/>

<img src="https://github.com/kliuengineering/SmartValve/blob/master/pics/ESPServer_result_2.png"/>

<img src="https://github.com/kliuengineering/SmartValve/blob/master/pics/ESPServer_result_3.png"/>

```bash
Please press Enter to continue...
Please press Enter to continue...
K66F - Received ets Jul 29 2019 12:21:46

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_d

K66F - Received rv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4916
load:0x40078000,len:1

K66F - Received 64�

ESP - Connecting to tovn


K66F - Received ESP - WiFi Connected! IP Address: 172.20.10.3
ESP - HTTP server started on port 80


K66F - SENDDATA command received
K66F - send {"sensor": 0, "value": 1640049856}
K66F - Received SENDDATA

K66F - Received ESP - Received HTTP GET - /sensor


actuator command received
K66F - Received {
    "actuator": 1,
    "command": 0
}ESP - Received HTTP POST - /actuator
```
