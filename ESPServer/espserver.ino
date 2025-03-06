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

IPAddress local_IP(172, 20, 10, 3);  // Choose an IP within your phone's AP range
IPAddress gateway(172, 20, 10, 1);   // Typically, the phone's IP is the gateway
IPAddress subnet(255, 255, 255, 0);  // Standard subnet mask


WebServer server(80);

TaskHandle_t task_loop2_for_k66f;

void on_connect_handler();
void sensor_handler();
void actuator_handler();
void not_found_handler();
void loop2_for_k66f(void* pvParameters);
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

  // Start a thread to read and write data to K66F
  xTaskCreatePinnedToCore(
    loop2_for_k66f,       /* Function to implement the task */
    "Task1",              /* Name of the task */
    10000,                /* Stack size in words */
    NULL,                 /* Task input parameter */
    0,                    /* Priority of the task */
    &task_loop2_for_k66f, /* Task handle. */
    0);                   /* Core where the task should run */
  delay(500);
}

void loop() {
  server.handleClient();
}

// this loop is to fake data that can potentially read and write data to K66F
void loop2_for_k66f(void* pvParameters) {
  for (;;) {
    sensor = random(0, 3);
    sensor_val = random(45, 60);
    delay(500);
  }
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
  ptr += "<title>Rice</title>\n";
  ptr += "<h1>Chicken Wings with Fried Rice</h1>\n";
  ptr += "<h3>Using WIFI Mode</h3>\n";
  ptr += "<p>Device IP Address: <strong>" + st + "</strong></p>";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}