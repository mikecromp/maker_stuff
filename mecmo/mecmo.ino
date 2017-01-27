#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <eBtn.h>                 //https://github.com/david1983/eBtn.git
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <fauxmoESP.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson.git
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager



//////////////////////////////////////
// CONSTANTS
//////////////////////////////////////
#define SERIAL_BAUD   115200
#define LED_PIN       LED_BUILTIN
#define RELAY_PIN     D5
#define BUTTON_PIN    D3

//////////////////////////////////////
// GLOBALS
//////////////////////////////////////
WiFiManager             wifiManager;
char                    device_name[34];
const char*             host = "mecmo-webupdate";
fauxmoESP               fauxmo;
ESP8266WebServer        httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
volatile int            relayState = LOW;
bool                    shouldSaveConfig = false; // Tell WiFi Manager not to save data
float                   btnPressDur; // Used for the eBtn
eBtn                    btn = eBtn(BUTTON_PIN);


//////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////
void setupHardware();
void wifiManagerSetup();

////////////////////////////////
void setup() {
  // setup the hardware needed
  setupHardware();

  // instantiate WiFiManager setup
  wifiManagerSetup();

  // startup the https server to handle software updates
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.begin(host);
  MDNS.addService("http", "tcp", 80);
  Serial.printf("\n\nHTTPUpdateServer ready!\n\nOpen http://%s/update in your browser\n", WiFi.localIP().toString().c_str());


  // setup the connections for communicating with Alexa
  Serial.println("\nSetuping up for Alexa!!\n");
  fauxmo.addDevice(device_name);
  fauxmo.onMessage(fauxmoCallback);

  // add the interrupt for button on the swithch
  //  attachInterrupt(BUTTON_PIN, relayToggle, RISING);
  // add the interrupt for button on the swithch
  attachInterrupt(BUTTON_PIN, resetWiFi, RISING);
}  // end setup()

////////////////////////////////
void loop() {
  // handle any HTTP requests
  httpServer.handleClient();

  // handle requests from Alexa
  fauxmo.handle();
}  // end loop()

////////////////////////////////
//callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
} // end saveConfigCallback

////////////////////////////////
void fauxmoCallback(uint8_t device_id, const char * fdevice_name, bool state) {
  Serial.printf("[MAIN] %s state: %s\n", fdevice_name, state ? "ON" : "OFF");

  if ( (strcmp(fdevice_name, device_name) == 0) ) {
    // adjust the relay immediately!
    if (state) {
      relayState = HIGH;
      Serial.println("Relay ON");
    } else {
      relayState = LOW;
      Serial.println("Relay OFF");
    }
    setRelay();
  }
} // end fauxmoCallback







