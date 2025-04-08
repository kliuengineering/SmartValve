// #include <WiFi.h>
// #include <WebServer.h>
// #include <ArduinoJson.h>

// using namespace std;

// /* Put your SSID & Password */
// const char* ssid = "tovn";            // Enter SSID here
// const char* password = "toan123123";  // Enter Password here

// /* Sensor and actuator*/
// int sensor = 1;
// int sensor_val = 1;

// IPAddress local_IP(172, 20, 10, 3);  // Choose an IP within your phone's AP range
// IPAddress gateway(172, 20, 10, 1);   // Typically, the phone's IP is the gateway
// IPAddress subnet(255, 255, 255, 0);  // Standard subnet mask

// WebServer server(80);

// void on_connect_handler();
// void sensor_handler();
// void actuator_handler();
// void not_found_handler();

// String SendHTML(String st);

// void setup() {
//   Serial.begin(115200);
//   pinMode(5, OUTPUT);  // set the LED pin mode

//   Serial.println();
//   Serial.println();

//   Serial.print("ESP - Connecting to ");
//   Serial.println(ssid);
//   if (!WiFi.config(local_IP, gateway, subnet)) {
//     Serial.println("ESP - Failed to configure static IP");
//   }

//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     for (int i = 0; i < 3 && WiFi.status() != WL_CONNECTED; i++) {
//       digitalWrite(5, HIGH);
//       delay(500);
//     }
//     if (WiFi.status() != WL_CONNECTED) Serial.println();
//   }
//   digitalWrite(5, LOW);

//   Serial.print("ESP - WiFi Connected! IP Address: ");
//   Serial.println(WiFi.localIP());


//   server.on("/", HTTP_GET, on_connect_handler);
//   server.on("/sensor", HTTP_GET, sensor_handler);
//   server.on("/actuator", HTTP_POST, actuator_handler);
//   server.onNotFound(not_found_handler);

//   server.begin();
//   Serial.println("ESP - HTTP server started on port 80");
// }

// void loop() {
//   server.handleClient();
// }

// void on_connect_handler() {
//   server.send(200, "text/html", SendHTML(WiFi.localIP().toString()));
// }

// void sensor_handler() {

//   // TODO: read data from the K66F and deserialize it for logging
//   Serial.write("SENDDATA");

//   // Wait briefly to allow the K66F time to respond.
//   delay(1000);

//   // Read the response from K66F (if available).
//   char k66fResponse[128];
//   size_t respBytes = 0;

//   if (Serial.available() > 0) {
//     respBytes = Serial.readBytes(k66fResponse, sizeof(k66fResponse) - 1);
//   }
//   if (respBytes > 0) {
//     k66fResponse[respBytes] = '\0';
//   } else {
//     strcpy(k66fResponse, "No response");
//   }

//   String json_st = k66fResponse;
//   Serial.println("ESP - Received HTTP GET - /sensor");

//   // TODO: forward it to client
//   server.send(200, "application/json", k66fResponse);
// }

// void actuator_handler() {
//   if (!server.hasArg("plain")) {
//     server.send(400, "text/plain", "Bad request: Body is empty");
//     return;
//   }

//   // TODO: read data from the client and deserialize it for logging
//   String body = server.arg("plain");
//   Serial.write(body.c_str());

//   Serial.println("ESP - Received HTTP POST - /actuator");
//   // TODO: confirm with client with a status
//   server.send(200, "application/json", "{\"status\":\"OK\"}");
// }

// void not_found_handler() {
//   server.send(404, "text/plain", "{\"status\":\"OK\"}, \"error\":\"Not Found\"}");
// }

// String SendHTML(String st) {
//   String ptr = "<!DOCTYPE html> <html>\n";
//   ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
//   ptr += "<title>Habibi</title>\n";
//   ptr += "<h1>h=Habibi The West</h1>\n";
//   ptr += "<h3>Using WIFI Mode</h3>\n";
//   ptr += "<p>Device IP Address: <strong>" + st + "</strong></p>";
//   ptr += "</body>\n";
//   ptr += "</html>\n";
//   return ptr;
// }

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

using namespace std;

/* Put your SSID & Password */
const char* ssid = "tovn";            // Enter SSID here
const char* password = "toan123123";  // Enter Password here

/* Sensor and actuator*/
int sensor = 1;
int sensor_val = 1;
float prevPressure = 0.0;
unsigned long prevTime = 0;  // Holds the timestamp of the previous sensor reading

IPAddress local_IP(172, 20, 10, 3);  // Choose an IP within your phone's AP range
IPAddress gateway(172, 20, 10, 1);   // Typically, the phone's IP is the gateway
IPAddress subnet(255, 255, 255, 0);  // Standard subnet mask

WebServer server(80);

void on_connect_handler();
void sensor_handler();
void actuator_handler();
void not_found_handler();


String requestK66FData();
float performPressureGradient(float currentPressure, float previousPressure);
void sendActuatorCommand(int actuator, int command);
void sensorTask(void* parameters);

String SendHTML(String st);

void setup() {
  Serial.begin(115200);
  pinMode(5, OUTPUT);  // set the LED pin mode

  Serial.println();
  Serial.println();

  Serial.print("ESP - Connecting to ");
  Serial.println(ssid);
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("ESP - Failed to configure static IP");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    for (int i = 0; i < 3 && WiFi.status() != WL_CONNECTED; i++) {
      digitalWrite(5, HIGH);
      delay(500);
    }
    if (WiFi.status() != WL_CONNECTED) Serial.println();
  }
  digitalWrite(5, LOW);

  Serial.print("ESP - WiFi Connected! IP Address: ");
  Serial.println(WiFi.localIP());


  server.on("/", HTTP_GET, on_connect_handler);
  server.on("/sensor", HTTP_GET, sensor_handler);
  server.on("/actuator", HTTP_POST, actuator_handler);
  server.onNotFound(not_found_handler);

  server.begin();
  Serial.println("ESP - HTTP server started on port 80");

  // Create and pin the sensorTask to Core 0 (for example).
  xTaskCreatePinnedToCore(
    sensorTask,    // Function to implement the task.
    "SensorTask",  // Name of the task.
    10000,         // Stack size in bytes.
    NULL,          // Parameter passed to the task.
    1,             // Priority of the task.
    NULL,          // Task handle.
    0              // Core where the task should run (0 or 1).
  );
}

void loop() {
  server.handleClient();
}

void on_connect_handler() {
  server.send(200, "text/html", SendHTML(WiFi.localIP().toString()));
}

void sensor_handler() {

  // TODO: read data from the K66F and deserialize it for logging
  Serial.write("SENDDATA");

  // Wait briefly to allow the K66F time to respond.
  delay(1000);

  // Read the response from K66F (if available).
  char k66fResponse[128];
  size_t respBytes = 0;

  if (Serial.available() > 0) {
    respBytes = Serial.readBytes(k66fResponse, sizeof(k66fResponse) - 1);
  }
  if (respBytes > 0) {
    k66fResponse[respBytes] = '\0';
  } else {
    strcpy(k66fResponse, "No response");
  }

  String json_st = k66fResponse;
  Serial.println("ESP - Received HTTP GET - /sensor");

  // TODO: forward it to client
  server.send(200, "application/json", k66fResponse);
}

void actuator_handler() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Bad request: Body is empty");
    return;
  }

  // TODO: read data from the client and deserialize it for logging
  String body = server.arg("plain");
  Serial.write(body.c_str());

  Serial.println("ESP - Received HTTP POST - /actuator");
  // TODO: confirm with client with a status
  server.send(200, "application/json", "{\"status\":\"OK\"}");
}

void not_found_handler() {
  server.send(404, "text/plain", "{\"status\":\"OK\"}, \"error\":\"Not Found\"}");
}

String SendHTML(String st) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Habibi</title>\n";
  ptr += "<h1>h=Habibi The West</h1>\n";
  ptr += "<h3>Using WIFI Mode</h3>\n";
  ptr += "<p>Device IP Address: <strong>" + st + "</strong></p>";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

// 1. Request sensor data from K66F.
// Sends "SENDDATA" to the K66F, waits for a response, and returns it as a String.
String requestK66FData() {
  Serial.write("SENDDATA");
  delay(1000);  // Allow time for K66F to respond.
  char response[128];
  size_t bytesRead = 0;
  if (Serial.available() > 0) {
    bytesRead = Serial.readBytes(response, sizeof(response) - 1);
  }
  if (bytesRead > 0) {
    response[bytesRead] = '\0';
    return String(response);
  }
  return String("No response");
}

// 2. Perform pressure gradient calculation.
// Takes current and previous pressure readings and returns their difference.
float performPressureGradient(float currentPressure, float previousPressure, float dt) {
  // Avoid division by zero
  if (dt <= 0) {
    return 0;
  }
  return (currentPressure - previousPressure) / dt;
}

// 3. Send actuator command to the K66F.
// Given an actuator (e.g., 1 for the servo) and a command (e.g., 1 to close, 0 to open),
// this function builds and sends a JSON command via Serial.
void sendActuatorCommand(int actuator, int command) {
  String actuatorCommand = "{\"actuator\":" + String(actuator) + ", \"command\":" + String(command) + "}";
  Serial.println("Sending actuator command to K66F:");
  Serial.println(actuatorCommand);
  Serial.write(actuatorCommand.c_str());  // Send the command.
}

// This task continuously reads the sensor value from the K66F, calculates the pressure gradient,
// and sends actuator commands accordingly.
void sensorTask(void* parameters) {
  while (1) {
    unsigned long currentTime = millis();

    // Read sensor data from K66F.
    String k66fResponse = requestK66FData();
    Serial.println("Sensor Task - Received from K66F:");
    Serial.println(k66fResponse);

    // Parse the JSON response.
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, k66fResponse);
    float currentPressure = 0.0;
    if (!error) {
      currentPressure = doc["pressure"];
    } else {
      Serial.println("Sensor Task - Failed to parse sensor JSON data");
    }

    // Only calculate dt and gradient if we have a valid previous reading.
    if (prevTime > 0) {
      float dt = (currentTime - prevTime) / 1000.0;  // Convert milliseconds to seconds.
      float gradient = performPressureGradient(currentPressure, prevPressure, dt);
      Serial.print("Sensor Task - dt (s): ");
      Serial.println(dt);
      Serial.print("Sensor Task - Pressure Gradient (hPa/sec): ");
      Serial.println(gradient);

      // Example threshold:
      // If gradient is less than -1.0 hPa/sec, send command to close valve.
      if (gradient < -1.0) {
        sendActuatorCommand(1, 1);  // actuator 1, command 1: close valve.
      }
      // If gradient is greater than +1.0 hPa/sec, send command to open valve.
      else if (gradient > 1.0) {
        sendActuatorCommand(1, 0);  // actuator 1, command 0: open valve.
      }
    }

    // Update previous pressure and time for the next calculation.
    prevPressure = currentPressure;
    prevTime = currentTime;

    // Sleep for 0.5 second.
    delay(500);
  }
}