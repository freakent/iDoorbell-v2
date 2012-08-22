/*
  Analog Input
 
 */

const unsigned int NORMAL_MODE = 0;
const unsigned int QUIET_MODE = 1;

const unsigned int XBEE_ENABLE_PIN = 8;
const unsigned int QUIET_MODE_LED = 13;
const unsigned long QUIET_DURATION = 10000;

const unsigned long PING_INTERVAL = 60000;
const unsigned int SENSOR_PIN = A1;    // select the input pin for the sensor

int sensorValue = 0;  // variable to store the value coming from the sensor
int amps = 0;
unsigned int mode = NORMAL_MODE; 
unsigned long quiet_alarm = 0;
unsigned long ping_alarm = 0;

void setup() {
  pinMode(QUIET_MODE_LED, OUTPUT);
  pinMode(XBEE_ENABLE_PIN, OUTPUT);
  digitalWrite(XBEE_ENABLE_PIN, HIGH);
  Serial.begin(9600);  
  setupXBee();
  ping_alarm = millis() + PING_INTERVAL;
}

void loop() {

  switch (mode) {
    case NORMAL_MODE:
    
      digitalWrite(QUIET_MODE_LED, LOW);

      // read the value from the sensor:
      sensorValue = analogRead(SENSOR_PIN);   

      amps = map(sensorValue, 0, 1023, -50, 50);  // Actual amps * 10 to remove decimal point
      
      if (amps < -10 || amps > 10) { // It's AC so could be +ve or -ve
        String msg = "DINGDONG ";
        xbeeSend(msg + sensorValue + " " + amps);
        mode = QUIET_MODE;
        quiet_alarm = millis() + QUIET_DURATION;
        return;
      }
      
      checkPingAlarm();
      
      break;
      
    case QUIET_MODE:
      
      digitalWrite(QUIET_MODE_LED, HIGH);

      if ((long)(millis() - quiet_alarm) >= 0 ) {
        mode = NORMAL_MODE; 
      }
      
      break;
      
  }
  
}
  
void setupXBee() {
  Serial.println("Initialising\r");
  delay(1000);
  
  while (!xbeeCommandMode()) {
    digitalWrite(QUIET_MODE_LED, HIGH);
    delay(500);
    digitalWrite(QUIET_MODE_LED, LOW);
    delay(500);
  }
  
  //xbeeCommand("ATRE\r"); // Factory defaults
  //xbeeCommand("ATID0\r"); // Extended PAN Id
  //xbeeCommand("ATSCFFFF\r"); // Scan Channels
  xbeeCommand("ATVR\r"); // Firmware Version
  
  xbeeCommand("ATDH0, DL0\r"); // Destination Address L/H 0/0 is coordinator
  xbeeCommand("ATAI\r"); // Association Ind (read only)
  xbeeCommand("ATCN\r"); // Exit command mode
}

void checkStatus() {  
  while (!xbeeCommandMode()) {
    digitalWrite(QUIET_MODE_LED, HIGH);
    delay(500);
    digitalWrite(QUIET_MODE_LED, LOW);
    delay(500);
  }
  xbeeCommand("ATAI\r"); // Association Ind (read only)
  xbeeCommand("ATCN\r"); // Exit command mode
}

boolean xbeeCommandMode() {
  delay(1000);
  Serial.print("+++");
  delay(1000);
  return checkResult();
}

boolean xbeeCommand(char cmd[]) {
  Serial.print(cmd);
  return checkResult();
}

/*
 * checkResult - Checks the result of sending an xbee command, returns either true or false
 */
boolean checkResult() {
  
  for (int x = 0; x < 30 && Serial.available() == 0; x ++) {
    delay(100);
  }
  
  char buffer[100] = "";
  for (int x = 0; x < 100 && Serial.available(); x++) {
      buffer[x] = Serial.read();
  }
  Serial.println(buffer);
  
  String result = String(buffer);
  Serial.println(result);
  
  if (result.equals("") || result.indexOf("ERR") >= 0) {
    return false;
  } else {
    return true;
  }  
}

void xbeeSend(String msg) {
  //checkStatus();
  int l = msg.length() + 1;
  char data[l];
  msg.toCharArray(data, l); 
  Serial.println(data);
  delay(500);
}

void checkPingAlarm() {
  unsigned long time = millis();
  if ((long)(time - ping_alarm) >= 0) {
    String ping_msg = "PING ";
    xbeeSend(ping_msg + time + " " + sensorValue);
    ping_alarm = time + PING_INTERVAL;
  } 
}



