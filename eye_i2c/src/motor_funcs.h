void setMotorState(uint8_t motor, boolean flag) {
  if (flag) {
    digitalWrite(motorPins[motor][3], HIGH);// enable 
  }
  else {
    digitalWrite(motorPins[motor][3], LOW); // disable
  }
}

void setMotorPosition(uint8_t motor, int position, uint8_t power) {
  int pwm_a, pwm_b, pwm_c;
  
  // if only it was this easy to constrain absolute power
  power = abs(constrain(power, 0, 255)); 

  // get number from the sin table, change amplitude from max
  pwm_a = (pwmSin[(position + currentStepA) % 360]) * (power / 255.0);
  pwm_b = (pwmSin[(position + currentStepB) % 360]) * (power / 255.0);
  pwm_c = (pwmSin[(position + currentStepC) % 360]) * (power / 255.0);

  analogWrite(motorPins[motor][0], pwm_a);
  analogWrite(motorPins[motor][1], pwm_b);
  analogWrite(motorPins[motor][2], pwm_c);
}

void initMotors() {
  for (int8_t x = 0; x < 4; x++) {
    pinMode(motorPins[x][0], OUTPUT);
    pinMode(motorPins[x][1], OUTPUT);
    pinMode(motorPins[x][2], OUTPUT);
    pinMode(motorPins[x][3], OUTPUT); 
    analogWriteFrequency(motorPins[x][0], PWM_SPEED);
    analogWriteFrequency(motorPins[x][1], PWM_SPEED);
    analogWriteFrequency(motorPins[x][2], PWM_SPEED);
    setMotorState(x, false);
  }
}

void updateMotor(uint8_t motor, int16_t target) {
  uint16_t slideVal = analogRead(slidePins[motor]);
  uint16_t position = motorPositions[motor];

  // get the value from the joystick if target = -1
  if (target == -1) { target = joyVals[motor]; }

  boolean moving = false;

  uint16_t delta = abs(target - slideVal);

  motorNow = millis(); 
  if(slideVal > slideMax[motor] || slideVal < slideMin[motor]) { // it's fucked
    Serial.printf("bad slideVal: %d motor: %d\n", slideVal, motor);
    Serial.printf(" min :: %d max :: %d\n", slideMin[motor], slideMax[motor]);
    setMotorState(motor, false);
  }
  else if(delta < MIN_DELTA) { // it's in the donut hole
    stopState[motor] = true;
    if ((motorNow - motorTime[motor]) > motorInterval) {
      // we were in the donut hole for a while
      setMotorState(motor, false);
    }
  } 
  else { 
    if(delta < MAX_DELTA) {
      // inside the donut, not in the donut hole
      if (stopState[motor] == true) { // it has been in the hole
	if ((motorNow - motorTime[motor]) > motorInterval) {
	  setMotorState(motor, false);
	}
      }
      else { // keep moving
	motorTime[motor] = millis();
	setMotorState(motor, true);
	if (slideVal < target) { position = (position == 359) ? 0 : position - 1; }
	else { position = (position == 359) ? 0 : position + 1; }
	setMotorPosition(motor, position, MAX_PWER);
	moving = true;
      }
    }
    else { // outside the donut, also keep moving
      motorTime[motor] = millis();
      setMotorState(motor, true);
      if (slideVal < target) { position = (position == 359) ? 0 : position - 1; }
      else { position = (position == 359) ? 0 : position + 1; }
      setMotorPosition(motor, position, MAX_PWER);
      stopState[motor] = false; // reset this thing.
      moving = true;
    }
  }

  if (moving == false) { motorStopped[motor]++;}

  slideVals[motor] = slideVal;
  motorPositions[motor] = position;
}

void updateMotors(int16_t target) {
  updateMotor(0, target);
  updateMotor(1, target);
  updateMotor(2, target);
  updateMotor(3, target);
}

