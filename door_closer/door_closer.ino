const int SWITCH_LED = 8;     // Use to turn on LED with switch is colosed
const int TIMER_LED = 12;     // Use to turn on LED indicating timer is active
const int TIMEOUT_LED = 13;   // Use to turn on LED when TIMEOUT occurs
const int MAG_SWITCH = 2;     // Use to monitor Reed Swithch
const int HOLD_SWITCH = 4;    // Use to read the hold switch
const int CLOSE_SWITCH = 7;   // Use to close door switch

const int TIMEOUT = 5000;     // set TIMEOUT value to 5000 mSec
const int OPEN = LOW;         // state when switch is open
const int CLOSED = HIGH;      // state when switch is closed 
const int ON = HIGH;          // LED ON
const int OFF = LOW;          // LED OFF

int magSwitchState = -1;      // Variable us to capture Reed switch state 0 is open, 1 is closed
int holdButtonState = -1;     
int oldState = -1;            // Previous state of the Reed switch saved when a state change occurs

int startTime = -1;           // used to capture time when switch closes
int delta = 0;                // delta between current and start times

// the setup routine runs once when you press reset:
void setup() {                
  Serial.begin(9600);
  // initialize the digital pin as an output.
  pinMode(SWITCH_LED, OUTPUT);
  pinMode(TIMER_LED, OUTPUT);
  pinMode(TIMEOUT_LED, OUTPUT);
  pinMode(MAG_SWITCH, INPUT);
  pinMode(HOLD_SWITCH, INPUT);
  pinMode(CLOSE_SWITCH, OUTPUT);

  digitalWrite(SWITCH_LED, OFF);  // Off
  digitalWrite(TIMER_LED, OFF);   // Off
  digitalWrite(TIMEOUT_LED, OFF); // Off
  digitalWrite(CLOSE_SWITCH, OFF); 
}  // end setup()

// the loop routine runs over and over again forever:
void loop() {
  
  magSwitchState = digitalRead(MAG_SWITCH);
Serial.print("m-state=");
Serial.println(magSwitchState);
  holdSwitchState = digitalRead(HOLD_SWITCH);
Serial.print("h-state=");
Serial.println(holdSwitchState);

  if (holdSwitchState = CLOSED) {  // hold button was pressed (button is Normally Open)
    digitalWrite(HOLD_LED, ON);
    delay(2000);
//    delay(5000);
  }  // end if hold switch closed
  else {  // hold switch is open
    digitalWrite(HOLD_LED, OFF);
    if (magSwitchState != oldState) {  // state of switch changed
Serial.print("oldstate=");
Serial.println(oldState);
      oldState = magSwitchState;
      
      if (magSwitchState == OPEN) { // switch has opened (switch is Normally Closed)
        digitalWrite(SWITCH_LED, ON);
        digitalWrite(TIMER_LED, ON);
        startTime = millis();
Serial.print("start=");
Serial.println(startTime);
      }  // end if switch has opened  
      else { // swith has closed
        digitalWrite(SWITCH_LED, OFF);
        digitalWrite(TIMER_LED, OFF);
        digitalWrite(TIMEOUT_LED, OFF);
        startTime = -1;
        delta = 0;
Serial.print("end timer: start=");
Serial.println(startTime);
      }  // end else switch has closed
    }  // end if state of switch changed
      
    if (startTime != -1) {  // timer is running
      delta = millis() - startTime;
Serial.print("delta=");
Serial.println(delta);
      if (delta > TIMEOUT) {  // timer expired
        digitalWrite(TIMER_LED, OFF);
        digitalWrite(TIMEOUT_LED, ON);
        digitalWrite(CLOSE_SWITCH, ON);
        delay(500);
        digitalWrite(TIMEOUT_LED, OFF);
        digitalWrite(CLOSE_SWITCH, OFF);
        startTime = -1;
        delta = 0;
      }  // end if timer expired
    }  // end if timer is running
  }  // end else holdSwitch open
    delay(1000);  // wait a second before looping again
}  // end loop()
