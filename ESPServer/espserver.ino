#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "NeuralNetwork.h"

// WiFi Credentials
const char* ssid = "Amir";
const char* password = "gotthat1";

// IP Configuration
IPAddress local_IP(172, 20, 10, 3);
IPAddress gateway(172, 20, 10, 1);
IPAddress subnet(255, 255, 255, 0);

// Web Server
WebServer server(80);

// Sensor/Actuator and Gradient Tracking
float prevPressure = 0.0;
unsigned long prevTime = 0;

// AI Model
NeuralNetwork model(3);
InputLayer input(1);
DenseLayer hidden(4, ActivationKind::Relu);
OutputLayer output(1, ActivationKind::Sigmoid);

// Function Prototypes
void on_connect_handler();
void sensor_handler();
void actuator_handler();
void not_found_handler();
String SendHTML(String st);
String requestK66FData();
float performPressureGradient(float currentPressure, float previousPressure, float dt);
void sendActuatorCommand(int actuator, int command);
void sensorTask(void * parameters);

void setup() {
  Serial.begin(115200);
  pinMode(5, OUTPUT);

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(5, LOW);
  Serial.println("\nESP - WiFi Connected!");

  // Web routes
  server.on("/", HTTP_GET, on_connect_handler);
  server.on("/sensor", HTTP_GET, sensor_handler);
  server.on("/actuator", HTTP_POST, actuator_handler);
  server.onNotFound(not_found_handler);
  server.begin();

  // Build AI model
  model.StackLayer(&input).StackLayer(&hidden).StackLayer(&output);
  model.Build();
  // TODO: Hardcode trained weights and biases here

  // Start Sensor Task
  xTaskCreatePinnedToCore(sensorTask, "SensorTask", 10000, NULL, 1, NULL, 0);
}

void loop() {
  server.handleClient();
}

void on_connect_handler() {
  server.send(200, "text/html", SendHTML(WiFi.localIP().toString()));
}

void sensor_handler() {
  String response = requestK66FData();
  server.send(200, "application/json", response);
}

void actuator_handler() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Bad request: Body is empty");
    return;
  }
  String body = server.arg("plain");
  Serial.write(body.c_str());
  server.send(200, "application/json", "{\"status\":\"OK\"}");
}

void not_found_handler() {
  server.send(404, "text/plain", "{\"error\":\"Not Found\"}");
}

String SendHTML(String st) {
  return "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Flood Monitor</title></head><body><h1>Flood Monitoring System</h1><p>IP Address: <strong>" + st + "</strong></p></body></html>";
}

String requestK66FData() {
  Serial.write("SENDDATA");
  delay(1000);
  char response[128];
  size_t bytesRead = 0;
  if (Serial.available() > 0) {
    bytesRead = Serial.readBytes(response, sizeof(response) - 1);
  }
  response[bytesRead] = '\0';
  return String(response);
}

float performPressureGradient(float currentPressure, float previousPressure, float dt) {
  return (dt > 0) ? (currentPressure - previousPressure) / dt : 0;
}

void sendActuatorCommand(int actuator, int command) {
  String cmd = "{\"actuator\":" + String(actuator) + ", \"command\":" + String(command) + "}";
  Serial.println(cmd);
  Serial.write(cmd.c_str());
}

void sensorTask(void * parameters) {
  for (;;) {
    unsigned long currentTime = millis();
    String response = requestK66FData();

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, response);
    float currentPressure = 0.0;
    if (!error) {
      currentPressure = doc["pressure"];
    } else {
      Serial.println("Failed to parse pressure JSON");
    }

    if (prevTime > 0) {
      float dt = (currentTime - prevTime) / 1000.0;
      float gradient = performPressureGradient(currentPressure, prevPressure, dt);
      Serial.print("Gradient: "); Serial.println(gradient);

      float inputVals[1] = {gradient};
      float* result = model.Predict(inputVals, 1);

      if (result[0] > 0.7) {
        sendActuatorCommand(1, 1); // High risk - close valve
      } else if (result[0] < 0.3) {
        sendActuatorCommand(1, 0); // Low risk - open valve
      }
    }

    prevPressure = currentPressure;
    prevTime = currentTime;
    delay(500);
  }
}
