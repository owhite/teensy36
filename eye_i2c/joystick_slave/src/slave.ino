#include <i2c_t3.h>

#define SERIAL_SPEED   115200
#define JOY_SLAVE_ADDRESS 0x09
#define PACKET_LEN 13

#define USB Serial

#define MEM_LEN 12
#define SERIAL_SPEED  115200

uint8_t dataBuf[PACKET_LEN]; 

uint8_t LEDPin = 13;

uint8_t btnPin1 = 5;
uint8_t btnPin2 = 6;
uint8_t joy1Pin  = 15; // yellow
uint8_t joy2Pin  = 14; // orange
uint8_t joy3Pin  = 17; // red
uint8_t joy4Pin  = 16; // brown

uint16_t joy1PinVal; uint16_t joy2PinVal;
uint16_t joy3PinVal; uint16_t joy4PinVal;

boolean blinkOn = false;
uint32_t blinkDelta = 0;
uint32_t blinkInterval = 200; 
uint32_t blinkNow;

String inputString = "";

void setup() {
  inputString.reserve(200);

  USB.begin(SERIAL_SPEED);

  pinMode(btnPin1, INPUT);
  pinMode(btnPin2, INPUT);
  pinMode(LEDPin, OUTPUT);

  Wire.begin(I2C_SLAVE, JOY_SLAVE_ADDRESS, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);

  Wire.onRequest(requestEvent);
}

void loop() {
  // checkUSBSerial();

  joy1PinVal = analogRead(joy1Pin); joy2PinVal = analogRead(joy2Pin);
  joy3PinVal = analogRead(joy3Pin); joy4PinVal = analogRead(joy4Pin);

  Serial.printf("%d :: %d\n", joy3PinVal, joy4PinVal);

  dataBuf[0] = '$'; 
  dataBuf[1] = digitalRead(btnPin1);
  dataBuf[2] = digitalRead(btnPin2);
  dataBuf[3] = highByte(joy1PinVal);  dataBuf[4] = lowByte(joy1PinVal);
  dataBuf[5] = highByte(joy2PinVal);  dataBuf[6] = lowByte(joy2PinVal);
  dataBuf[7] = highByte(joy3PinVal);  dataBuf[8] = lowByte(joy3PinVal);
  dataBuf[9] = highByte(joy4PinVal); dataBuf[10] = lowByte(joy4PinVal);
  dataBuf[11] = '\n';

  blinkNow = millis();
  if ((blinkNow - blinkDelta) > blinkInterval) {
    blinkOn = !blinkOn;
    digitalWrite(LEDPin, blinkOn);
    blinkDelta = blinkNow;
  }
}

void requestEvent() { Wire.write(dataBuf, PACKET_LEN); }

void checkUSBSerial() {
  while (USB.available()) {
    char inChar = (char)USB.read();
    inputString += inChar;
    if (inChar == '\n') {
      handle_cmd();
    }
  }
}

void handle_cmd() {
  inputString.trim(); // removes beginning and ending white spaces
  int idx = inputString.indexOf(' ');   
  String cmd = inputString.substring(0, idx); 
  String value = inputString.substring(idx + 1);

  if ((cmd.length() > 0)) {
    if (cmd.equals("probe_device")) {
      USB.println("joy stick");
    }
    if (cmd.equals("idle")) {
      USB.println(cmd);
    }
    if (cmd.equals("report_joystick")) {
      USB.println(cmd);
    }
    inputString = "";
  }
}

