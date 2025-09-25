#include <WiFi.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>


// Function prototypes
void togglePin();
void enablePin1();
void disablePin1();
void enablePin2();
void disablePin2();
void displayPage(WiFiClient &client);
void sendSpeed(WiFiClient &client);

// Network Credentials
const char* ssid = "Bugs in a Jug";
const char* password = "Fungi2024";

// Web server port number
WiFiServer server(80);

// HTTP request variable
String header;

// Pin output state variables
String statePin13 = "off";
String stateEnable1 = "enabled";
String stateEnable2 = "enabled";

const int cmdPin = 13;
const int cmdEnablePin1 = 12;
const int cmdEnablePin2 = 27;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Timeout time in milliseconds
const long timeoutTime = 2000;

// GPS signal status
bool gpsSignalValid = false;
// Speed in mph
float speedMPH = 0.0;

// Define the serial connection to GPS module
HardwareSerial GPS_Serial(1); // Use UART port 1 on ESP32

// Define the TinyGPS++ object
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  GPS_Serial.begin(9600, SERIAL_8N1, 16, 17); // Initialize GPS serial communication

  pinMode(cmdPin, OUTPUT);
  pinMode(cmdEnablePin1, OUTPUT);
  pinMode(cmdEnablePin2, OUTPUT);
  digitalWrite(cmdPin, LOW); // Set the pin LOW initially to turn it off
  digitalWrite(cmdEnablePin1, HIGH); // Set the enable pin HIGH initially to enable
  digitalWrite(cmdEnablePin2, HIGH); // Set the enable pin HIGH initially to enable

  WiFi.softAP(ssid, password);

  // Print IP address and start web server
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

void loop() {
  while (GPS_Serial.available() > 0) {
    if (gps.encode(GPS_Serial.read())) {
      if (gps.speed.isValid()) {
        speedMPH = gps.speed.mph(); // Update speed if valid
        Serial.print("Speed: ");
        Serial.print(speedMPH);
        Serial.println(" MPH");
      }
      if (gps.satellites.isValid() && gps.satellites.value() >= 2) {
        gpsSignalValid = true; // Update signal validity
      } else {
        gpsSignalValid = false;
      }
    }
  }

  WiFiClient client = server.available(); // Listen for incoming clients

  if(client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      // loop while the client is connected
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turn GPIO on and off
            if (header.indexOf("GET /toggle") >= 0) {
              togglePin();
            } else if (header.indexOf("GET /enable1") >= 0) { // Corrected URL for enabling pin 1
              enablePin1();
            } else if (header.indexOf("GET /disable1") >= 0) { // Corrected URL for disabling pin 1
              disablePin1();
            } else if (header.indexOf("GET /enable2") >= 0) { // Corrected URL for enabling pin 2
              enablePin2();
            } else if (header.indexOf("GET /disable2") >= 0) { // Corrected URL for disabling pin 2
              disablePin2();
            } else if (header.indexOf("GET /speed") >= 0) { // New endpoint to fetch speed data
              sendSpeed(client);
            }

            // Display the HTML web page
            displayPage(client);

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void togglePin() {
  if (statePin13 == "off") {
    statePin13 = "on";
    digitalWrite(cmdPin, HIGH); // Turn the pin ON (3.3V)
  } else {
    statePin13 = "off";
    digitalWrite(cmdPin, LOW); // Turn the pin OFF (0V)
  }
}

void enablePin1() {
  stateEnable1 = "enabled";
  digitalWrite(cmdEnablePin1, HIGH); // Enable the pin
}

void disablePin1() {
  stateEnable1 = "disabled";
  digitalWrite(cmdEnablePin1, LOW); // Disable the pin
}

void enablePin2() {
  stateEnable2 = "enabled";
  digitalWrite(cmdEnablePin2, HIGH); // Enable the pin
}

void disablePin2() {
  stateEnable2 = "disabled";
  digitalWrite(cmdEnablePin2, LOW); // Disable the pin
}

void displayPage(WiFiClient &client) {
  // Display the HTML web page
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");

  // CSS to style the buttons
  client.println("<style>html { font-family: monospace; display: inline-block; margin: 20px auto; text-align: center;}");
  client.println(".button-container { margin-top: 20px; margin-bottom: 10px; display: grid; grid-template-columns: 1fr 1fr; gap: 20px; justify-content: center; align-items: center; }"); // Adjusted CSS for button-container
  client.println(".button { border: none; color: white; padding: 14px 20px;");
  client.println("text-decoration: none; font-size: 24px; margin: 2px; cursor: pointer;}");
  client.println(".button1 { background-color: yellowgreen; }"); // Button style for the bottom two buttons, initially green
  client.println(".button2 { background-color: gray; }"); // Button style for the top button, initially gray
  client.println(".button p { margin: 0; }"); // Ensure text is centered horizontally within the buttons
  client.println(".status { margin-top: 20px; font-size: 20px; }"); // Added style for status text
  client.println("</style></head>");

  client.println("<body><h1>Bugs in a Jug</h1>");

  // Main System On/Off button
  client.print("<p><a href=\"/toggle\"><button class=\"button");
  client.print(statePin13 == "off" ? " button2" : " button1"); // Change class based on pin state
  client.println("\" style=\"width: 200px; border-radius: 10px;\">"); // Apply rounded corners
  client.print(statePin13 == "off" ? "OFF" : "ON");
  client.println("</button></a></p>");

  // Container for the smaller buttons
  client.println("<div class=\"button-container\">");

  // Enabled button 1
  client.print("<div style=\"grid-column: 1 / span 1;\"><p style=\"font-size: 22px;\">Bug Juice</p><a href=\"");
  client.print(stateEnable1 == "enabled" ? "/disable1" : "/enable1"); // Toggle URL based on pin state
  client.print("\"><button class=\"button button1");
  client.print(stateEnable1 == "enabled" ? "" : " button2"); // Change class based on pin state
  client.println("\" style=\"width: 70%; border-radius: 10px;\">"); // Apply rounded corners
  client.print(stateEnable1 == "enabled" ? "Enabled" : "Disabled");
  client.println("</button></a></div>");

  // Enabled button 2
  client.print("<div style=\"grid-column: 2 / span 1;\"><p style=\"font-size: 22px;\">Molasses</p><a href=\"");
  client.print(stateEnable2 == "enabled" ? "/disable2" : "/enable2"); // Toggle URL based on pin state
  client.print("\"><button class=\"button button1");
  client.print(stateEnable2 == "enabled" ? "" : " button2"); // Change class based on pin state
  client.println("\" style=\"width: 70%; border-radius: 10px;\">"); // Apply rounded corners
  client.print(stateEnable2 == "enabled" ? "Enabled" : "Disabled");
  client.println("</button></a></div>");

  client.println("</div>"); // Close button-container

  // Display GPS status and speed
  client.println("<div style=\"text-align: center; margin-top: 0.5in;\">");

  // Speed display in colored box
  client.print("<div style=\"display: inline-block; position: relative; background-color: ");
  client.print(gpsSignalValid ? "yellowgreen" : "red"); // Set color based on GPS signal status
  client.println("; padding: 15px; border-radius: 10px;\">"); // Apply rounded corners
  client.print("<span style=\"font-size: 80px; font-weight: bold; color: black;\" id=\"speedValue\">");
  client.print(speedMPH);
  client.println("</div>");

  // Abbreviation "MPH" outside the box to the right
  client.println("<span style=\"font-family: 'Digital-07', monospace; font-size: 80px; position: absolute; bottom: 270px; right: 280px;\">MPH</span>");
  client.println("</div>");

  // JavaScript for updating speed display every 500ms
  client.println("<script>");
  client.println("setInterval(function() {");
  client.println("  fetch('/speed')"); // Fetch speed data from server
  client.println("    .then(response => response.json())"); // Parse JSON response
  client.println("    .then(data => {");
  client.println("      const speed = data.speed;"); // Extract speed value from JSON
  client.println("      document.getElementById('speedValue').textContent = speed;"); // Update speed display
  client.println("    });");
  client.println("}, 500);"); // Update speed every 500ms
  client.println("</script>");

  client.println("</body></html>");
}

void sendSpeed(WiFiClient &client) {
  // Send speed data as JSON with HTTP headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json"); // Specify the content type as JSON
  client.println("Connection: close"); // Close the connection after sending the response
  client.println(); // Empty line to indicate the end of headers
  
  // Create a JSON object containing the speed value
  StaticJsonDocument<200> doc;
  doc["speed"] = speedMPH;
  
  // Serialize the JSON object to a string
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send the JSON string as the response body
  client.print(jsonString);
}



