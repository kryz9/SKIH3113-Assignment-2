Objective:
In this assignment, you will develop a sensor-based system using an ESP8266 microcontroller.
The system will utilize EEPROM for reading and writing Wi-Fi configuration data, device ID, and
the last output status (LED/RELAY/SERVO/etc.). You will implement an Access Point (AP) mode
web interface for configuring these parameters, and upon saving the configuration, the ESP8266
will reload with the saved settings and restore the last output status and set to the output
component.

Instructions:
1. Design of your project
• Connections to your input and output component using fritzing or any other design
software.

2. Setup and Initialization:
• Configure the ESP8266 to start in AP mode if no WiFi configuration is found in EEPROM.
• Create a web server accessible in AP mode to input WiFi credentials, device ID, and
initial status of an output device (e.g., LED or Relay).

3. EEPROM Handling:
• Implement functions to read and write WiFi credentials, device ID, and last output
status to EEPROM.
• Ensure that the data read from EEPROM is validated and defaults are used if no valid
data is available.

4. Web Configuration Interface:
• Develop an HTML web page served by the ESP8266 in AP mode to collect WiFi
credentials, device ID, and initial output status.
• Include form fields for SSID, Password, Device ID, and an option to set the output
status (ON/OFF for an LED or Relay).

5. Configuration Saving and Loading:
• Save the provided configuration to EEPROM when the user submits the form.
• Restart the ESP8266 and attempt to connect to the provided WiFi network using the
saved credentials.
• If the connection is successful, load the last output status and apply it to the output
device (e.g., turn on/off an LED or Relay).

6. Normal Operation:
• If the ESP8266 starts with valid WiFi credentials in EEPROM, it should connect to the
WiFi and set the output device to the last saved status.
• Implement the logic to handle reconnection attempts if the WiFi connection fails.
