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
  //  digitalWrite(RELAY_PIN, relayState);
  Serial.printf("Writing relay state %d\n", relayState);
} // end setRelay

