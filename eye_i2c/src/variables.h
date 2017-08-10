#define MAX_PWER 100
#define MAX_DELTA 40
#define MIN_DELTA 10
#define PWM_SPEED 50000
#define SERIAL_SPEED   115200
#define FORWARD 1
#define REVERSE 2
#define JOY_LEN 12
#define JOY Serial1
#define RA_SAMPLES 20
#define SLIDE_PACKET_SIZE 8
#define MAX_RECORD_LENGTH 5000

uint8_t slideArray[MAX_RECORD_LENGTH][SLIDE_PACKET_SIZE];

typedef struct slideData {
  uint16_t slideVal1;
  uint16_t slideVal2;
  uint16_t slideVal3;
  uint16_t slideVal4;
} t_slideData_def;

typedef union {
  t_slideData_def  slideData;
  uint8_t slidePacket[SLIDE_PACKET_SIZE];
} t_slidePacket_def;

t_slidePacket_def slideStore;

// sd file system stuff
SdFatSdio sd;
File file;
char sdFileName[SD_FILE_NAME_SIZE];

uint8_t menuSelectNum = 0;
uint8_t oldMenuNum = 0;
boolean inMenuZone = false;

char *iconNames[] = {"Joystick", "Run", "Record", "Play", "Save"};
uint8_t menuSelectStates[] = {E_RUNNING, E_RUNNING, E_START_RECORD, E_START_PLAYBACK, E_RUNNING};

elapsedMillis elapsedBtn1Time;
uint16_t btn1Delay = 200;
uint8_t btn1State = LOW;
boolean btn1Pressed = false;

elapsedMillis elapsedBtn2Time;
uint16_t btn2Delay = 200;
uint8_t btn2State = LOW;
boolean btn2Pressed = false;

elapsedMicros elapsedRecordTime;
uint32_t recordInterval = 5000;
uint32_t recordTime;
uint16_t recordCount = 0;

uint16_t playbackCount = 0;
uint8_t  pbMotor;
uint16_t pbSlideVal;
uint32_t pbTime;
uint32_t pbInterval = 5000; // microsecs

Adafruit_SSD1306 display(OLED_RESET);
boolean dontUpdateDisplay = false;
uint8_t xPos;
uint8_t yPos;

uint32_t motorTime[4] = {0, 0, 0, 0};
uint32_t motorInterval = 800;
uint32_t motorNow;

uint16_t dataTotal;

uint8_t dataBuf[PACKET_LEN]; 

uint16_t joyVals[4];
uint16_t rawJoyVals[4];

uint16_t slideMin[] = {200, 200, 345, 300};
uint16_t slideMax[] = {860, 800, 850, 740};

// uint8_t slidePins[] = {A22, A21, 34, 33}; // L to R, device facing you.
uint8_t slidePins[] = {33, A21, A22, 34}; // L to R, device facing you.
uint16_t slideVals[4] = {0, 0, 0, 0};
uint16_t oldSlideVals[4] = {0, 0, 0, 0};
boolean stopState[] = {false,false,false,false};

uint8_t LEDPin = 13;

boolean joyToggle = true;

String inputString = "";

uint8_t state = E_IDLE;
uint8_t oldState = 0;
uint8_t oldDisplay = 0;

uint8_t joyState1 = 0;
uint8_t joyState2 = 0;

uint16_t motorPositions[] = {0, 0, 0, 0};
uint16_t moveToPositions[] = {0, 0, 0, 0};
uint16_t motorStopped[] = {0, 0, 0, 0};

byte motorPins[4][4]={
  // pins for IN1, IN2, IN3 and ENable
  {35, 36, 38, 37}, // M3
  {10, 29, 30, 31}, // M4
  {21, 22, 20, 23}, // M1
  {14, 16, 17, 15} // M2
};

int32_t sineArraySize = 360;
int32_t phaseShift = sineArraySize / 3; 
int32_t currentStepA = 0;
int32_t currentStepB = currentStepA + phaseShift;
int32_t currentStepC = currentStepB + phaseShift;

const int pwmSin[] = {128, 132, 136, 140, 143, 147, 151, 155, 159, 162, 166,
		      170, 174, 178, 181, 185, 189, 192, 196, 200, 203, 207,
		      211, 214, 218, 221, 225, 228, 232, 235, 238, 239, 240,
		      241, 242, 243, 244, 245, 246, 247, 248, 248, 249, 250,
		      250, 251, 252, 252, 253, 253, 253, 254, 254, 254, 255,
		      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		      255, 254, 254, 254, 253, 253, 253, 252, 252, 251, 250,
		      250, 249, 248, 248, 247, 246, 245, 244, 243, 242, 241,
		      240, 239, 238, 239, 240, 241, 242, 243, 244, 245, 246,
		      247, 248, 248, 249, 250, 250, 251, 252, 252, 253, 253,
		      253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255,
		      255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253,
		      253, 252, 252, 251, 250, 250, 249, 248, 248, 247, 246,
		      245, 244, 243, 242, 241, 240, 239, 238, 235, 232, 228,
		      225, 221, 218, 214, 211, 207, 203, 200, 196, 192, 189,
		      185, 181, 178, 174, 170, 166, 162, 159, 155, 151, 147,
		      143, 140, 136, 132, 128, 124, 120, 116, 113, 109, 105,
		      101, 97, 94, 90, 86, 82, 78, 75, 71, 67, 64, 60, 56, 53,
		      49, 45, 42, 38, 35, 31, 28, 24, 21, 18, 17, 16, 15,
		      14, 13, 12, 11, 10, 9, 8, 8, 7, 6, 6, 5, 4, 4, 3, 3, 3,
		      2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
		      3, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14,
		      15, 16, 17, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 8,
		      7, 6, 6, 5, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1,
		      1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 6, 7, 8,
		      8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 21, 24, 28, 31,
		      35, 38, 42, 45, 49, 53, 56, 60, 64, 67, 71, 75, 78, 82,
		      86, 90, 94, 97, 101, 105, 109, 113, 116, 120, 124};

