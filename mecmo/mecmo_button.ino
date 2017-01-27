// ***** Btn stuff ***** //
void setBtn() {
  btn.setThreshold(8000);
  btn.on("press", pressFunc);
  btn.on("hold", holdFunc);
  btn.on("release", releaseFunc);
  btn.on("long", longPressFunc);
  //setting the interrupt on btn pin to react on change state
  attachInterrupt(BUTTON_PIN, pin_ISR, CHANGE);
} // end setBtn

//function to handle the interrupt event
void pin_ISR() {
  btn.handle();
} // end pin_ISR

//callbacks functions
void pressFunc() {
  btnPressDur = millis();
  Serial.println("Btn pressed: " + String(btnPressDur / 1000) + " seconds");
} // end pressFunc

void releaseFunc() {
  relayToggle();
  Serial.println("Btn released after " + String((millis() - btnPressDur) / 1000) + " seconds");
} // end releaseFunc

void longPressFunc() {
  wifiManager.resetSettings();
  wifiManagerSetup();
  Serial.println("Btn released after a long press of " + String((millis() - btnPressDur) / 1000) + " seconds");
} // end longPressFunc

void holdFunc() {
  relayToggle();
  Serial.println("Btn hold for: " + String((millis() - btnPressDur) / 1000) + " seconds");
} // end holdFunc
// ***** End Btn Stuff ***** //
