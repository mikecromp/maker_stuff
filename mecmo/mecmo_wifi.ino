////////////////////////////////
void resetWiFi() {
  Serial.println('\nButton Pressed\nResetting WiFi Settings...\n');
  wifiManager.resetSettings();
  wifiManagerSetup();
} // end resetWiFi()

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
  }  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_device_name("device", "device name", device_name, 32);
  WiFiManagerParameter custom_text("<p>Enter your device name here.<br>&quot;Alexa, turn <i>device name</i> on.&quot;</p>");

  wifiManager.addParameter(&custom_text);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_device_name);

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality(50);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  String str = "mecmo" + String(ESP.getChipId());
  const char * ssid = str.c_str();
  if (!wifiManager.autoConnect(ssid, "mikerocks")) {
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
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUILTIN_LED, HIGH);  // turn off LED with voltage HIGH
    delay(300);                       // wait half a second
    digitalWrite(BUILTIN_LED, LOW);   // turn on LED with voltage LOW
    delay(800);                      // wait one second
  }
  digitalWrite(BUILTIN_LED, HIGH);  // turn off LED with voltage HIGH
} // end wifiManagerSetup
