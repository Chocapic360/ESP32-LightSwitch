#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
HTTPClient http;

// Parse JSON response
DynamicJsonDocument doc(1024);

#define B1 26
#define B2 25
#define B3 33
#define B4 32

const char *ssid = "wifi-ssid";
const char *password = "wifi-password";

const char *apiKey = "govee-api-key"; // get api key from govee
const char *url = "https://developer-api.govee.com/v1/devices";
const char *control_endpoint = "https://developer-api.govee.com/v1/devices/control";
const String &state_endpoint = "https://developer-api.govee.com/v1/devices/state";


// devices that we want to turn on and off
const char *devicesToControl[] = { "[1] Smart LED Bulb", "[2] Smart LED Bulb", "[3] Smart LED Bulb", "[4] Smart LED Bulb" };
bool getDevicesSuccess = false;


struct DeviceState {
  String powerState;
  bool online;
};

void setup() {
  Serial.begin(115200);

  pinMode(B1, INPUT_PULLUP);
  pinMode(B2, INPUT_PULLUP);
  pinMode(B3, INPUT_PULLUP);
  pinMode(B4, INPUT_PULLUP);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(1000);
  getDevices(); // Fetch Govee device information
  delay(5000);
}

void loop() {
  // Switch 1 - turns off lights
  if(digitalRead(B1) == LOW){
    Serial.println("--- switch off ----");
    printDevices("off");
    delay(5000);
  }
  if(digitalRead(B2) == LOW){
    Serial.println("--- switch purple ----");
    colorDevices(148,0,211,false);
    delay(5000);
  }
  if(digitalRead(B3) == LOW){
    Serial.println("--- switch red ----");
    colorDevices(255,0,0,true);
    delay(5000);
  }
  if(digitalRead(B4) == LOW){
    Serial.println("--- switch white ----");
    colorDevices(255,255,255,false);
    delay(5000);
  }
}

void getDevices() {
  HTTPClient http;

  Serial.print("Sending GET request to Govee API...");

  // Your Govee API URL
  http.begin(url);

  // Set headers
  http.addHeader("Govee-API-Key", apiKey);

  // Send the request
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
      deserializeJson(doc, payload);

      getDevicesSuccess = true;  // a successful API call

      int numDevicesToControl = sizeof(devicesToControl) / sizeof(devicesToControl[0]);


      // Extract and print device information
      JsonArray devices = doc["data"]["devices"];
      for (JsonObject device : devices) {
        const char *deviceName = device["deviceName"];
        const char *device_address = device["device"];
        for (int i = 0; i < numDevicesToControl; i++) {

          if (strcmp(deviceName, devicesToControl[i]) == 0) {

            const char *model = device["model"];
            bool controllable = device["controllable"];
            bool retrievable = device["retrievable"];

            Serial.println("Device Name: " + String(deviceName) + " " + String(device_address));
            Serial.println("Model: " + String(model));
            Serial.println("Controllable: " + String(controllable ? "Yes" : "No"));
            Serial.println("Retrievable: " + String(retrievable ? "Yes" : "No"));
            Serial.println("------");
            break;
          }
        }
      }
    }
  } else {
    Serial.printf("[HTTP] GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  // Free resources
  http.end();
}

void printDevices(const String &action) {
  // Check if the API call was successful before printing devices
  if (getDevicesSuccess) {
    // Extract and print device information
    JsonArray devices = doc["data"]["devices"];
    for (JsonObject device : devices) {
      const char *deviceName = device["deviceName"];
      const char *device_address = device["device"];
      const char *model = device["model"];
      bool controllable = device["controllable"];
      bool retrievable = device["retrievable"];

      DeviceState state = getDeviceState(device_address, model);

      Serial.println("Device Name: " + String(deviceName));
      Serial.println("Model: " + String(model));

      Serial.println("Power State: " + state.powerState);
      Serial.println("Online: " + String(state.online ? "Yes" : "No"));
      if(state.online) {
        // then we can either turn on or off

        if(action != state.powerState) {
          controlDevice(device_address, model, "turn", action);
          Serial.println("[ACTION] Turn " + String(action));
        } else {
          Serial.println("do nothing...");
        }
      }

      // Serial.println("Controllable: " + String(controllable ? "Yes" : "No"));
      // Serial.println("Retrievable: " + String(retrievable ? "Yes" : "No"));
      Serial.println("------");
    }
  } else {
    Serial.println("API call was not successful. Check the previous logs for details.");
  }
}

void colorDevices(const int r, const int g, const int b, bool dim) {
  // Check if the API call was successful before printing devices
  if (getDevicesSuccess) {
    // Extract and print device information
    JsonArray devices = doc["data"]["devices"];
    for (JsonObject device : devices) {
      const char *deviceName = device["deviceName"];
      const char *device_address = device["device"];
      const char *model = device["model"];
      bool controllable = device["controllable"];
      bool retrievable = device["retrievable"];

      DeviceState state = getDeviceState(device_address, model);

      Serial.println("Device Name: " + String(deviceName));
      Serial.println("Model: " + String(model));

      Serial.println("Power State: " + state.powerState);
      Serial.println("Online: " + String(state.online ? "Yes" : "No"));
      if(state.online) {
        // then we can either turn on or off
        if(state.powerState == "off"){
          controlDevice(device_address, model, "turn", "on");
          delay(1000);
        }
        controlRGB(device_address, model,r,g,b);
        delay(100);
        if(dim){
          controlBrightness(device_address, model, "brightness", 1);
        }
        else{
          controlBrightness(device_address, model, "brightness", 100);
        }
        Serial.println("[ACTION] Coloring LEDS");
      }

      // Serial.println("Controllable: " + String(controllable ? "Yes" : "No"));
      // Serial.println("Retrievable: " + String(retrievable ? "Yes" : "No"));
      Serial.println("------");
    }
  } else {
    Serial.println("API call was not successful. Check the previous logs for details.");
  }
}

void controlBrightness(const String &device, const String &model, const String &cmdName, const int &cmdValue) {
  HTTPClient http;

  Serial.print("[CONTROL DEVICE]");
  http.begin(control_endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Govee-API-Key", apiKey);

  // Create JSON request body
  StaticJsonDocument<200> jsonBody;
  jsonBody["device"] = device;
  jsonBody["model"] = model;

  JsonObject cmdObject = jsonBody.createNestedObject("cmd");
  cmdObject["name"] = cmdName;
  cmdObject["value"] = cmdValue;

  // Serialize JSON to string
  String jsonString;
  serializeJson(jsonBody, jsonString);

  // Send the request
  int httpCode = http.PUT(jsonString);

  if (httpCode > 0) {
    // HTTP header has been sent and the response status code received
    Serial.printf("[HTTP] PUT... code: %d\n", httpCode);

    // File found at the server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] PUT request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  // Free resources
  http.end();
}

void controlRGB(const String &device, const String &model, const int r, const int g, const int b) {
  HTTPClient http;

  Serial.print("[CONTROL DEVICE]");
  http.begin(control_endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Govee-API-Key", apiKey);

  // Create JSON request body
  StaticJsonDocument<200> jsonBody;
  jsonBody["device"] = device;
  jsonBody["model"] = model;

  JsonObject cmdObject = jsonBody.createNestedObject("cmd");
  cmdObject["name"] = "color";
  JsonObject colorObject = cmdObject.createNestedObject("value");
  colorObject["r"] = r;
  colorObject["g"] = g;
  colorObject["b"] = b;

  // Serialize JSON to string
  String jsonString;
  serializeJson(jsonBody, jsonString);

  // Send the request
  int httpCode = http.PUT(jsonString);

  if (httpCode > 0) {
    // HTTP header has been sent and the response status code received
    Serial.printf("[HTTP] PUT... code: %d\n", httpCode);

    // File found at the server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] PUT request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  // Free resources
  http.end();
}

void controlDevice(const String &device, const String &model, const String &cmdName, const String &cmdValue) {
  HTTPClient http;

  Serial.print("[CONTROL DEVICE]");
  http.begin(control_endpoint);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Govee-API-Key", apiKey);

  // Create JSON request body
  StaticJsonDocument<200> jsonBody;
  jsonBody["device"] = device;
  jsonBody["model"] = model;

  JsonObject cmdObject = jsonBody.createNestedObject("cmd");
  cmdObject["name"] = cmdName;
  cmdObject["value"] = cmdValue;

  // Serialize JSON to string
  String jsonString;
  serializeJson(jsonBody, jsonString);

  // Send the request
  int httpCode = http.PUT(jsonString);

  if (httpCode > 0) {
    // HTTP header has been sent and the response status code received
    Serial.printf("[HTTP] PUT... code: %d\n", httpCode);

    // File found at the server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] PUT request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  // Free resources
  http.end();
}

DeviceState getDeviceState(const String &device, const String &model) {
  HTTPClient http;

  Serial.print("[GET STATE]");

  String apiUrl = state_endpoint + "?device=" + device + "&model=" + model;
  http.begin(apiUrl);

  // Set headers
  http.addHeader("Govee-API-Key", apiKey);

  DeviceState result;  // Create a struct to store the result

  // Send the request
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);

      // Parse JSON response
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      // Extract device state information
      JsonObject data = doc["data"];
      result.powerState = data["properties"][1]["powerState"].as<String>();
      result.online = data["properties"][0]["online"];
    }
  } else {
    Serial.printf("[HTTP] GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  // Free resources
  http.end();

  return result;
}
