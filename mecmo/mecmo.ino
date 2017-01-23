#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <fauxmoESP.h>
#include <DNSServer.h>
#include <ArduinoJson.h>      //https://github.com/bblanchon/ArduinoJson.git


//////////////////////////////////////
// CONSTANTS
//////////////////////////////////////
#define SERIAL_BAUD   115200
#define LED_PIN     LED_BUILTIN
#define RELAY_PIN     D5
#define BUTTON_PIN    D3

//////////////////////////////////////
// GLOBALS
//////////////////////////////////////
char          device_name[34];
fauxmoESP         fauxmo;
ESP8266WebServer    httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
volatile int      relayState = LOW;
bool          shouldSaveConfig = false; // Tell WiFi Manager not to save data

//////////////////////////////////////
// FUNCTIONS
//////////////////////////////////////
void setupHardware();

////////////////////////////////
void setup() {
  // setup the hardware needed
  setupHardware();

  // instantiate WiFiManager setup
  wifiManagerSetup();

  // startup the https server to handle software updates
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  Serial.print("HTTPUpdateServer ready! Open http://"); Serial.print(WiFi.localIP()); Serial.println("/update in your browser\n");


  // setup the connections for communicating with Alexa
//  fauxmo.addDevice(device_name);
//  fauxmo.onMessage(fauxmoCallback);

  // add the interrupt for button on the swithch
  attachInterrupt(BUTTON_PIN, relayToggle, RISING);
}  // end setup()

////////////////////////////////
void loop() {
  // handle any HTTP requests
  httpServer.handleClient();
}  // end loop()

////////////////////////////////
// setup hardware
void setupHardware() {
  // Init serial port and clean garbage
  Serial.begin(SERIAL_BAUD);
  Serial.println();

  Serial.println("After connection, ask Alexa/Echo to 'turn Device Name on' or 'off'");
  Serial.println();
  Serial.printf("Using onboard LED at pin: %d\n", LED_PIN);
  Serial.printf("Using pin %d to control the relay.\n", RELAY_PIN);
  pinMode(LED_PIN, OUTPUT);  // initialize onboard LED as output
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  setRelay();
} // end setupHardware

////////////////////////////////
//callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
} // end saveConfigCallback

////////////////////////////////
void fauxmoCallback(const char * fdevice_name, bool state) {
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

////////////////////////////////
void relayToggle() {
  if (relayState == LOW) {
    relayState = HIGH;
  } else {
    relayState = LOW;
  }
  setRelay();
} // end relayToggle

////////////////////////////////
void setRelay() {
  digitalWrite(RELAY_PIN, relayState);
} // end setRelay

////////////////////////////////
void wifiManagerSetup() {
  //clean FS, for testing
//  SPIFFS.format();  // Uncomment to reformat SPIFFS.  Use if changing anything to do with stored data

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(device_name, json["device_name"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_device_name("device", "device name", device_name, 32);
  WiFiManagerParameter custom_text("<p>Enter your device name here.<br>&quot;Alexa, turn <i>device name</i> on.&quot;</p>");

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.addParameter(&custom_text);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_device_name);

  //reset settings - for testing
//  wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality(80);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  String str = "mecmo" + String(ESP.getChipId());
//  const char * ssid = str.c_str();
//  Serial.println("Device ssid is:");
//  Serial.println(ESP.getChipId());
//  Serial.println(String(ESP.getChipId()));
//  Serial.println(str);
//  Serial.println(ssid);
//  if (!wifiManager.autoConnect(ssid, "joerocks")) {
  wifiManager.setAPStaticIPConfig(IPAddress(192,168,1,225), IPAddress(192,168,10,1), IPAddress(255,255,255,0));
  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(device_name, custom_device_name.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["device_name"] = device_name;
    Serial.print("Device is named: ");
    Serial.println(device_name);

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  // Blink LED three times to show connection complete
  for (int i=0; i<3; i++) {
    digitalWrite(BUILTIN_LED, HIGH);  // turn off LED with voltage HIGH
    delay(100);                       // wait half a second
    for (int j=0; j<5; j++) {
    digitalWrite(BUILTIN_LED, LOW);   // turn on LED with voltage LOW
    delay(300);                      // wait one second
    digitalWrite(BUILTIN_LED, HIGH);  // turn off LED with voltage HIGH
    delay(100);                       // wait half a second
    }
  }
  digitalWrite(BUILTIN_LED, HIGH);  // turn off LED with voltage HIGH
} // end wifiManagerSetup


