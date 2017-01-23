#define DOOR_OPEN_LED 13  // Use to control LED with switch is colosed indicating timer is active
#define HOLD_LED 12       // Use to control LED indicating Hold Open state (toggles with HOLD_BUTTON press) 
#define MAG_SWITCH 2      // Use to monitor Reed Swithch
#define HOLD_BUTTON 4     // Use to read the hold button
#define CLOSE_SWITCH 7    // Use to close door switch

#define TIMEOUT 5*1000         // time to wait before closing the garage door
#define CLOSE_TRIGGER_TIME 500 // time to keep relay open
#define WAIT_AFTER_CLOSE 3*1000  // time to wait after triggering door to close
#define POLL_DELAY 500         // time to wait between polls

#define OPEN LOW          // state when switch is open
#define CLOSED HIGH       // state when switch is closed 
#define ON HIGH           // LED ON
#define OFF LOW           // LED OFF
#define RELAY_ON 1        // Realy On state is active low
#define RELAY_OFF 0       // Relay Off state is high

int magSwitchState = -1;     // Reed switch state - senses that the door is open
int holdButtonState = -1;    // Hold button state - toggles whether door should be held open
int holdDoorOpen = 0;        // Should the door be kept open

int timerStarted = 0;           // used to capture time when switch closes
int delta = 0;               // delta between current and start times
//////////////////////////////////////////////////////////////////////////////////////////
//
// the setup routine runs once when you press reset:
void setup() {                

  digitalWrite(DOOR_OPEN_LED, OFF);
  digitalWrite(HOLD_LED, OFF);
  digitalWrite(CLOSE_SWITCH, RELAY_OFF); 

  // initialize the digital pin as an output.
  pinMode(MAG_SWITCH, INPUT_PULLUP);
  pinMode(HOLD_BUTTON, INPUT);

  pinMode(DOOR_OPEN_LED, OUTPUT);
  pinMode(HOLD_LED, OUTPUT);
  pinMode(CLOSE_SWITCH, OUTPUT);

}  // end setup()


//////////////////////////////////////////////////////////////////////////////////////////
//
// the loop routine runs over and over again forever:
void loop() {
//  Read the mag swith to see if the door is open and read the hold button to see if the 
//  door should be held open or not
  magSwitchState = digitalRead(MAG_SWITCH);
  holdButtonState = digitalRead(HOLD_BUTTON);

  if (holdButtonState == CLOSED) {  // hold button was pressed (button is Normally Open)
    //  Toggle the button state
    holdDoorOpen = !holdDoorOpen;
    //  set the LED on the button appropriately
    digitalWrite(HOLD_LED, holdDoorOpen);
  }  // end if hold switch closed

  if (!holdDoorOpen) {  // the door should not be held open
    // if the mag switch opened, the door is open and the timer is not running
    // turn on the LED and start the timer
    if (magSwitchState == OPEN) { 
      // if timer is not running yet, start it and turn on the LED
      if (!timerStarted) { 
        digitalWrite(DOOR_OPEN_LED, ON);
        timerStarted = millis();
      } // end if timer is not running
      else {
      }
    }  // end if switch is open
    else { // swith has closed (door has closed)
      // turn off the LED and clear out the timer
      digitalWrite(DOOR_OPEN_LED, OFF);
      timerStarted = 0;
      delta = 0;
    }  // end else switch has closed

    // if the timer is running check for timeout and close door if expired
    if (timerStarted) {  
      // compute the time since the timer started
      delta = millis() - timerStarted;
      if (delta > TIMEOUT) {  // timer expired
        // turn on the relay for 1/2 second then turn back off to close the garage door
        digitalWrite(CLOSE_SWITCH, ON);
        delay(CLOSE_TRIGGER_TIME);
        digitalWrite(CLOSE_SWITCH, OFF);

        delay(WAIT_AFTER_CLOSE);
        
        // reset the timer
        timerStarted = 0;
        delta = 0;
      }  // end if timer expired
    }  // end if timer is running
  }  // end if not hold door open
  else { //the door should be held open
    // check to see if the timer was started before the hold button was pressed
    if (timerStarted) {  // timer is running
      // reset the timer
      timerStarted = 0;
      delta = 0;
    }  // end if timer is running
  }  // end if hold door open

  delay(POLL_DELAY);  // wait a second before looping again
}  // end loop()

