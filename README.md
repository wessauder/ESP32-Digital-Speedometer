# ESP32-Digital-Speedometer
Utilizes a tinyGPS++ to report speed in MPH to a locally hosted webpage. Additionally, the webpage contains buttons to control digital outputs on the ESP32

System Components:  
1 GPS Module   
1 ESP32    
1 Web interface    

This project combines GPS speed measurement with a web-based control dashboard. The ESP32 acts as both the data processor and the WiFi access point. A GPS module continuously feeds data into the ESP32 over a dedicated UART. The TinyGPS++ library decodes this data and extracts speed (in miles per hour) and satellite validity. If a valid signal is present, the current speed is calculated and stored. At the same time, the ESP32 hosts a HTTP server. Any device connected to the ESP32’s access point can open the dashboard in a web browser. The dashboard provides:

System Toggle: Switch the main output (Pin 13) ON or OFF.

Independent Controls: Enable/disable two individual outputs (Pin 12) and (Pin 27).

GPS Speed Display: Large-format speed indicator with color-coded status: Green when GPS is valid, Red when GPS signal is lost

Live Updates: JavaScript requests the /speed endpoint every 500 ms. The ESP32 responds with JSON containing the latest speed, which updates the on-screen display in real time.

The ESP32 tracks each client session, processes HTTP GET requests to toggle pins or fetch speed data, and responds with both HTML for the control page and JSON for live updates.
