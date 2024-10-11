#include <Arduino.h>

// Select your modem:
#define TINY_GSM_MODEM_SIM800

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set Software Serial for AT commands (to the module)
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(53, 51);  // RX, TX

// Increase RX buffer to capture the entire response
#define TINY_GSM_RX_BUFFER 650

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints
#define TINY_GSM_DEBUG SerialMon
// #define LOGGING  // <- Logging is for the HTTP library


// Set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials
const char apn[]      = "safaricom";       // Replace with your APN
const char gprsUser[] = "saf";               // GPRS user (if any)
const char gprsPass[] = "data";               // GPRS password (if any)

// Server details (updated)
const char server[]   = "arduino.free.beeceptor.com";  // Known public API
const char resource[] = "/todos";         // Resource for a simple GET request
const int  port       = 80;             // HTTP port (no SSL/TLS)

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
TinyGsm        modem(SerialAT);
TinyGsmClient  client(modem);   // Using non-secure client for HTTP (port 80)
HttpClient     http(client, server, port);

void setup() {
  // Set console baud rate
  SerialMon.begin(9600);
  delay(10);

  // Start Software Serial for SIM800L communication
  SerialAT.begin(9600);
  delay(6000);

  // Restart the modem
  SerialMon.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);


  // Unlock SIM card with a PIN, if necessary
  if (GSM_PIN && modem.getSimStatus() != 3) { 
    modem.simUnlock(GSM_PIN); 
  }

}

void loop() {
  // Wait for network connection
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) { 
    SerialMon.println("Network connected"); 
  }

  // GPRS connection
  SerialMon.print(F("Connecting to GPRS with APN: "));
  SerialMon.println(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isGprsConnected()) { 
    SerialMon.println("GPRS connected"); 
    SerialMon.println(modem.getLocalIP());

    SerialMon.print(F("Performing HTTP POST request... "));
    DynamicJsonDocument postDoc(1024);
    postDoc["key1"] = "xyvie";
    postDoc["key2"] = "lyons";
    String postBody;
    serializeJson(postDoc, postBody);
    String contentType = "application/json";
    
      // send the POST request
  http.post(resource, contentType, postBody);
    // read the status code and body of the response
  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);



    http.stop();
  }else{
    SerialMon.print("Modem disconnected");
  }


  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));


}