// this code designed to work on PCBs with onboard L6234 drivers
// I tried to do this working through the serial and then gave up on it. 

#include <i2c_t3.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include "SdFat.h"
#include "definitions.h"
#include "icon_data.h"
#include "variables.h"
#include "SD_handlers.h"
#include "motor_funcs.h"

uint8_t i = 0;
uint16_t j = 0;

void setup() {
  inputString.reserve(200);

  randomSeed(analogRead(0));

  Serial.begin(SERIAL_SPEED);

  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_1200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.clearDisplay();

  pinMode(LEDPin, OUTPUT);

  if (!sd.begin()) { sd.initErrorHalt(); }

  initMotors();
}

void loop() {
  checkUSBSerial();
  everyLoop();
  if (i > 5) { getJoystickData(); i = 0; }
  i++;

  switch (state) {
  case E_IN_MENU:
    if (rawJoyVals[2] < 300) {
      if (inMenuZone == false) { menuSelectNum++; }
      if (menuSelectNum >= ICON_MAX) { menuSelectNum = 0; }
      inMenuZone = true;
    }
    else { inMenuZone = false; }

    if (menuSelectNum != oldMenuNum) {
      Serial.printf("here: %d\n", menuSelectNum);
      display.clearDisplay();
      display.setTextColor(WHITE);
  
      display.setTextSize(1); display.setCursor(0,5); display.print(stateNames[state]);
      display.setTextSize(1); display.setCursor(0,54); display.print(iconNames[menuSelectNum]);
      
      display.drawBitmap(34, 0, icon_bmp[menuSelectNum], ICON_WIDTH, ICON_HEIGHT, 1);

      display.display();
      // state = menuSelectStates[menuSelectNum];
    }
    oldMenuNum = menuSelectNum;

    break;

  case E_START_RECORD:
    recordCount = 0;
    elapsedRecordTime = 0;
    recordTime = recordInterval;
    state = E_RECORDING;
    break;

  case E_RECORDING:
    if (recordCount > MAX_RECORD_LENGTH - 1) {
      state = E_STOP_RECORD;
      break;
    }

    if (elapsedRecordTime > recordTime) {
      uint8_t motor = 0;
      slideStore.slideData.slideVal1 = slideVals[0];
      slideStore.slideData.slideVal2 = slideVals[1];
      slideStore.slideData.slideVal3 = slideVals[2];
      slideStore.slideData.slideVal4 = slideVals[3];
      for(uint8_t i = 0; i < SLIDE_PACKET_SIZE; i++) {
	slideArray[recordCount][i] = slideStore.slidePacket[i];
      }

      recordCount++;
      recordTime += recordInterval;
    }
    updateMotors(-1);
    break;

  case E_STOP_RECORD:
    state = E_RUNNING;
    break;

  case E_RUNNING:
    setDisplay(DISP_JOYSTICK);
    updateMotors(-1); 
    break;

  case E_START_PLAYBACK:
    playbackCount = 0;
    elapsedRecordTime = 0;
    pbTime = pbInterval;
    state = E_PLAYING;
    break;

  case E_PLAYING:
    if (playbackCount > recordCount || playbackCount > MAX_RECORD_LENGTH) {
      state = E_STOP_PLAYBACK;
      break;
    }

    if (elapsedRecordTime > pbTime) { // must be done, move on.
      for(uint8_t i = 0; i < SLIDE_PACKET_SIZE; i++) {
	slideStore.slidePacket[i] = slideArray[playbackCount][i];
      }
      slideVals[0] = slideStore.slideData.slideVal1;
      slideVals[1] = slideStore.slideData.slideVal2;
      slideVals[2] = slideStore.slideData.slideVal3;
      slideVals[3] = slideStore.slideData.slideVal4;

      pbTime += pbInterval;
      playbackCount++;
    }

    updateMotor(0, slideStore.slideData.slideVal1);
    updateMotor(1, slideStore.slideData.slideVal2);
    updateMotor(2, slideStore.slideData.slideVal3);
    updateMotor(3, slideStore.slideData.slideVal4);
    break;

  case E_STOP_PLAYBACK:
    // recordCount = 0;
    state = E_START_PLAYBACK;
    break;

  case E_REPORT_SLIDE:
    Serial.printf("slide %d %d %d %d\n",
		  analogRead(slidePins[0]),
		  analogRead(slidePins[1]),
		  analogRead(slidePins[2]),
		  analogRead(slidePins[3]));
    delay(300);
    state = E_REPORT_SLIDE;
    break;

  case E_SAVE:
    getRandomFileName(sdFileName);  
    writeSDFile(sdFileName, SD_FILE_NAME_SIZE);
    randomFileSelect(sdFileName);
    break;

  case E_REPORT_JOYSTICK:
    Serial.printf("states %d %d\n", joyState1, joyState2);
    Serial.printf("joy %d %d %d %d\n", joyVals[0], joyVals[1], joyVals[2], joyVals[3]);
    delay(800);
    break;

  default:
    Serial.printf("Unknown state %d\n", state);
    break;
  }

}

// housekeeping things
void everyLoop() {

  if (btn1Pressed && elapsedBtn1Time > btn1Delay) {
    if (state == E_IDLE) { state = E_RUNNING; }
    else { state = E_IDLE; }
    btn1State == LOW;
    btn1Pressed = false;
  }
  if (btn1State == HIGH) { elapsedBtn1Time = 0; btn1Pressed = true;}

  if (btn2Pressed && elapsedBtn2Time > btn2Delay) {
    if (state == E_IDLE || state == E_RUNNING) {
      Serial.printf("in menu\n");
      state = E_IN_MENU;

      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setTextSize(1); display.setCursor(0,5); display.print(stateNames[state]);
      display.setTextSize(1); display.setCursor(0,54); display.print(iconNames[menuSelectNum]);
      display.display();

      inMenuZone = false;
      // this not a good relationship between this point and what
      // menuselectnum should be
      menuSelectNum = 0;
    }
    else if (state == E_IN_MENU) {
      Serial.printf("selecting: %d\n", menuSelectStates[menuSelectNum]);
      state = menuSelectStates[menuSelectNum];
      oldState = state + 1;
    }
    else if (state == E_RECORDING) { 
      Serial.println("stop R");
      state = E_STOP_RECORD;
    }
    else if (state == E_PLAYING) { // master may have moved us to playing
      // this not really occur - if we're in playback you wouldnt hit the button
      Serial.println("stop PB");
      state = E_STOP_PLAYBACK;
    }
    else {}

    btn2State == LOW;
    btn2Pressed = false;
  }
  if (btn2State == HIGH) { elapsedBtn2Time = 0; btn2Pressed = true;}
}

void setDisplay(uint8_t d) {

  if ( state == oldState && d == oldDisplay) { return; }

  display.clearDisplay();
  display.setTextColor(WHITE);
  
  display.setTextSize(1); display.setCursor(0,5); display.print(stateNames[state]);
  display.setTextSize(1); display.setCursor(0,54); display.print(iconNames[d]);
  
  display.display();
  oldState = state;
  oldDisplay = d;
}

void checkUSBSerial() { 
  while (Serial.available()) {
    char inChar = (char)Serial.read();
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
      Serial.println("eye driver");
    }
    if (cmd.equals("run")) {
      Serial.println("running");
      state = E_RUNNING;
    }
    if (cmd.equals("report_state")) {
      Serial.println(state);
    }
    if (cmd.equals("purge")) {
      Serial.println("purging");
      // purge();
    }
    if (cmd.equals("report_slide")) {
      Serial.println(cmd);
      state = E_REPORT_SLIDE;
    }
    if (cmd.equals("report_joystick")) {
      Serial.println(cmd);
      state = E_REPORT_JOYSTICK;
    }
    if (cmd.equals("idle")) {
      Serial.println(cmd);
      state = E_IDLE;
    }
    inputString = "";
  }
}

void getJoystickData () {

  Wire.requestFrom(JOY_SLAVE_ADDRESS, PACKET_LEN, I2C_STOP);
  uint8_t i = 0;
  while(Wire.available()) { dataBuf[i] = Wire.readByte(); i++; }
  if (dataBuf[0] = '$' && dataBuf[PACKET_LEN-1] == '\n') {
    
    btn1State = dataBuf[1];
    btn2State = dataBuf[2];

    // motor3 :: top lid
    rawJoyVals[0] = word(dataBuf[3], dataBuf[4]); 
    joyVals[0] = map(rawJoyVals[0], 1, 1024, slideMin[0], slideMax[0]);
    joyVals[0] = constrain(joyVals[0], slideMin[0], slideMax[0]);

    // motor1 :: lower lid
    rawJoyVals[1] = word(dataBuf[5], dataBuf[6]); 
    joyVals[1] = map(rawJoyVals[1], 1, 1024, slideMax[1], slideMin[1]);
    joyVals[1] = constrain(joyVals[1], slideMin[1], slideMax[1]);

    // motor1 :: eye L / R
    rawJoyVals[2] = word(dataBuf[7], dataBuf[8]); 
    joyVals[2] = map(rawJoyVals[2], 1, 1024, slideMin[2], slideMax[2]);
    joyVals[2] = constrain(joyVals[2], slideMin[2], slideMax[2]);

    // motor2 :: eye U / D
    rawJoyVals[3] = word(dataBuf[9], dataBuf[10]); 
    joyVals[3] = map(rawJoyVals[3], 1, 1024, slideMin[3], slideMax[3]);
    joyVals[3] = constrain(joyVals[3], slideMin[3], slideMax[3]);

    // Serial.printf("joy %d %d %d %d\n", joyVals[0], joyVals[1], joyVals[2], joyVals[3]);
    // delay(800);

  }

}
