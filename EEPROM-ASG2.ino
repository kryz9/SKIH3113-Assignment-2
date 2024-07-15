#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LED_PIN 2 // GPIO2 (D4 on NodeMCU)
#define RESET_PIN 0 // GPIO0 (D3 on NodeMCU) for reset button

struct Config {
  char ssid[32];
  char password[32];
  char deviceId[16];
  bool lastOutputStatus;
};

Config config;
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP); // Set reset pin as input with internal pullup
  EEPROM.begin(512);

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();

  // Check if reset button is pressed during startup
  if (digitalRead(RESET_PIN) == LOW) {
    Serial.println("Reset button pressed. Clearing WiFi credentials...");
    clearWiFiCredentials(); // Clear WiFi credentials from EEPROM
    delay(1000); // Debounce delay
  }

  loadConfig(); // Load configuration from EEPROM

  if (String(config.ssid) == "") {
    startAPMode(); // Start in AP mode if no WiFi credentials are stored
  } else {
    connectToWiFi(); // Connect to WiFi using stored credentials
  }
}

void loop() {
  server.handleClient();
  
  // Update LED based on output status
  if (config.lastOutputStatus) {
    // Blink LED every 1 second
    digitalWrite(LED_PIN, HIGH); // Output status ON
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  } else {
    digitalWrite(LED_PIN, HIGH); // Output status OFF
  }
}

void clearWiFiCredentials() {
  // Clear WiFi credentials stored in EEPROM
  for (int i = 0; i < sizeof(Config); ++i) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  Serial.println("WiFi credentials cleared from EEPROM");
}

void initializeConfig() {
  strcpy(config.ssid, "AIMANBT2");
  strcpy(config.password, "mariaahmad@123");
  strcpy(config.deviceId, "default");
  config.lastOutputStatus = false; // Default to OFF state
  saveConfig();
}

void startAPMode() {
  // Start WiFi in AP mode
  WiFi.softAP("ESP8266_Config");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Display AP mode on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("AP Mode");
  display.print("IP: ");
  display.println(IP);
  display.setCursor(0, 20);
  display.print("Status: ");
  display.println(config.lastOutputStatus ? "ON" : "OFF");
  display.display();

  startWebServer(); // Start web server for configuration
}

void connectToWiFi() {
  // Connect to WiFi using stored credentials
  WiFi.begin(config.ssid, config.password);
  Serial.print("Connecting to WiFi");

  // Display connecting status on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi");
  display.display();

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    Serial.print(".");

    // Display connecting dots on OLED
    display.print(".");
    display.display();
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" connected!");
    display.println(" connected!");
    display.display();
    updateLED(); // Update LED based on output status
    
    // Wait for 5 seconds and then restart AP mode
    delay(5000);
    startAPMode();
  } else {
    Serial.println(" connection failed!");
    display.println(" connection failed!");
    display.display();
    // Handle connection failure, maybe return to AP mode or retry
    // For simplicity, returning to AP mode in case of failure
    startAPMode();
  }
}

void updateLED() {
  // Update LED based on output status
  if (config.lastOutputStatus) {
    digitalWrite(LED_PIN, LOW);  // Output status ON
  } else {
    digitalWrite(LED_PIN, HIGH); // Output status OFF
  }
}

void loadConfig() {
  // Load configuration from EEPROM
  EEPROM.get(0, config);
  if (String(config.ssid) == "" || String(config.deviceId) == "") {
    // If no valid configuration found, initialize with default values
    initializeConfig();
  }
}

void saveConfig() {
  // Save configuration to EEPROM
  EEPROM.put(0, config);
  EEPROM.commit();
}

void startWebServer() {
  server.on("/", HTTP_GET, []() {
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-EVSTQN3/azprG1Anm3QDgpJLIm9Nao0Yz1ztcQTwFspd3yD65VohhpuuCOmLASjC" crossorigin="anonymous">
        <title>ESP8266 Configuration</title>
      </head>
      <body>
        <div class="container mt-5">
          <div class="card">
            <div class="card-header">
              ESP8266 Configuration
            </div>
            <div class="card-body">
              <form id="configForm" action="/save" method="POST">
                <div class="form-group">
                  <label for="ssid">SSID</label>
                  <input type="text" class="form-control" id="ssid" name="ssid" value="{{ssid}}" required>
                </div>
                <div class="form-group">
                  <label for="password">Password</label>
                  <input type="password" class="form-control" id="password" name="password" value="{{password}}" required>
                </div>
                <div class="form-group">
                  <label for="deviceId">Device ID</label>
                  <select class="form-control" id="deviceId" name="deviceId" required>
                    <option value="default" {{selected_default}}>Default</option>
                    <option value="device1" {{selected_device1}}>Device 1</option>
                    <option value="device2" {{selected_device2}}>Device 2</option>
                    <option value="device3" {{selected_device3}}>Device 3</option>
                  </select>
                </div>
                <p></p>
                <div class="form-check form-switch">
                  <input class="form-check-input" type="checkbox" id="status_on" name="status_on" {{checked_on}} onclick="handleCheckboxChange(this)">
                  <label class="form-check-label" for="status_on">ON</label>
                </div>
                <div class="form-check form-switch">
                  <input class="form-check-input" type="checkbox" id="status_off" name="status_off" {{checked_off}} onclick="handleCheckboxChange(this)">
                  <label class="form-check-label" for="status_off">OFF</label>
                </div>
                <p></p>
                <button type="submit" class="btn btn-primary">Save</button>
                <p></p>
              </form>
              <br>
              <h4>Stored Configuration</h4>
              <table class="table">
                <thead>
                  <tr>
                    <th scope="col">SSID</th>
                    <th scope="col">Device ID</th>
                    <th scope="col">Output Status</th>
                  </tr>
                </thead>
                <tbody>
                  <tr>
                    <td>{{ssid}}</td>
                    <td>{{deviceId}}</td>
                    <td>{{outputStatus}}</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
        
        <!-- Bootstrap Modal -->
        <div class="modal fade" id="configModal" tabindex="-1" aria-labelledby="configModalLabel" aria-hidden="true">
          <div class="modal-dialog">
            <div class="modal-content">
              <div class="modal-header">
                <h5 class="modal-title" id="configModalLabel">Configuration Saved</h5>
                <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
              </div>
              <div class="modal-body">
                <div class="alert alert-success" role="alert">
                  Configuration saved successfully. The device will restart shortly.
                </div>
              </div>
            </div>
          </div>
        </div>

        <script src="https://cdn.jsdelivr.net/npm/@popperjs/core@2.9.2/dist/umd/popper.min.js" integrity="sha384-IQsoLXl5PILFhosVNubq5LC7Qb9DXgDA9i+tQ8Zj3iwWAwPtgFTxbJ8NT4GN1R8p" crossorigin="anonymous"></script>
        <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/js/bootstrap.min.js" integrity="sha384-cVKIPhGWiC2Al4u+LWgxfKTRIcfu0JTxR+EQDz/bgldoEyl4H0zUF0QKbrJ0EcQF" crossorigin="anonymous"></script>
        <script>
        function handleCheckboxChange(checkbox) {
          if (checkbox.id === "status_on" && checkbox.checked) {
            document.getElementById("status_off").checked = false;
          } else if (checkbox.id === "status_off" && checkbox.checked) {
            document.getElementById("status_on").checked = false;
          }

          if (checkbox.id === "status_on" && !checkbox.checked) {
            document.getElementById("status_off").checked = true;
          } else if (checkbox.id === "status_off" && !checkbox.checked) {
            document.getElementById("status_on").checked = true;
          }
        }

        document.getElementById('configForm').addEventListener('submit', function(e) {
          e.preventDefault();
          fetch('/save', {
            method: 'POST',
            body: new FormData(this)
          }).then(response => {
            if (response.ok) {
              var myModal = new bootstrap.Modal(document.getElementById('configModal'), {});
              myModal.show();
              setTimeout(() => {
                location.reload();
              }, 3000);
            }
          });
        });
        </script>
      </body>
      </html>
    )rawliteral";

    // Replace placeholders with actual data
    html.replace("{{ssid}}", config.ssid);
    html.replace("{{password}}", config.password);
    html.replace("{{deviceId}}", config.deviceId);
    html.replace("{{outputStatus}}", config.lastOutputStatus ? "ON" : "OFF");

    // Set selected option in device ID dropdown
    String selected_default = "";
    String selected_device1 = "";
    String selected_device2 = "";
    String selected_device3 = "";

    if (strcmp(config.deviceId, "default") == 0) {
      selected_default = "selected";
    } else if (strcmp(config.deviceId, "device1") == 0) {
      selected_device1 = "selected";
    } else if (strcmp(config.deviceId, "device2") == 0) {
      selected_device2 = "selected";
    } else if (strcmp(config.deviceId, "device3") == 0) {
      selected_device3 = "selected";
    }

    html.replace("{{selected_default}}", selected_default);
    html.replace("{{selected_device1}}", selected_device1);
    html.replace("{{selected_device2}}", selected_device2);
    html.replace("{{selected_device3}}", selected_device3);

    // Set checked status for radio buttons
    String checked_on = config.lastOutputStatus ? "checked" : "";
    String checked_off = !config.lastOutputStatus ? "checked" : "";

    html.replace("{{checked_on}}", checked_on);
    html.replace("{{checked_off}}", checked_off);

    server.send(200, "text/html", html);
  });

  server.on("/save", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String deviceId = server.arg("deviceId");
    bool status_on = server.hasArg("status_on");
    bool status_off = server.hasArg("status_off");

    // Ensure mutual exclusivity
    if (status_on && !status_off) {
      config.lastOutputStatus = true; // ON
    } else {
      config.lastOutputStatus = false; // OFF
    }

    ssid.toCharArray(config.ssid, 32);
    password.toCharArray(config.password, 32);
    deviceId.toCharArray(config.deviceId, 16);

    saveConfig();

    // Update LED immediately after saving configuration
    updateLED();

    // Simple response after saving
    server.send(200, "text/plain", "Configuration Saved");

    // Restart the device after a delay
    delay(2000);
    ESP.restart();
  });

  server.begin();
  Serial.println("HTTP server started");
}

