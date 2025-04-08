# Smart Valve Shield for FRDM-K66F
- Authours
    - Amir
    - Kevin
    - Toan

## Preliminary Product Rendering
![ProductImage](/pics/product.png)

## Peripherals On the Shield
**Note: refer to the detailed BOM for specific components.**
1. USB-C power input
2. ESP32S module (38 pins)
3. Class AB amplifier
4. RGB LED module
5. Pressure sensor Qwiic adapter
6. Header pins for servo motor
7. Various pin sockets
8. Various passive components

## Hardware
The design is simple - we want to maximize the real estate of the shield so it becomes a sandbox tool for education purposes. The project about SmartValve is just a simple example of showing you its use case for education. To be fair - K66F is a development platform and is naive for commercial implementations.

### Circuits
![Circuits](/pics/circuits.png)

### Routing
![Routing](/pics/pcb.png)

## Software

### Installations
1. If you are using Windows, then your machine may require an USB driver for the ESP32 MCU. You can download it here [USB Driver for Windows](Tools/CP210x_Universal_Windows_Driver.zip)

2. Follow the [ESP32 Markdown Documentation](./ESPServer/espserver.md) for server-side details.

### Data Logging Interface
![DataUI1](/pics/ESPServer_result_1.png)
![DataUI1](/pics/ESPServer_result_2.png)
![DataUI1](/pics/ESPServer_result_3.png)

## Future Work
1. Exploring radio-frequency applications.
2. Implementing TinyML on ESP to carry out more mature edge computing.