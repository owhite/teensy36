#define errorHalt(msg) {Serial.println(F(msg)); SysCall::halt();}
#define SLIDE_PACKET_SIZE 8
#define MAX_RECORD_LENGTH 5000
#define SD_FILE_NAME_SIZE 20
#define SD_FILE_KEY 'Z'

#define JOY_SLAVE_ADDRESS 0x09
#define PACKET_LEN 12
#define OLED_RESET 4

// eyeball states
#define E_IDLE             1  // no movement
#define E_RUNNING          2  // eye is moving, probably in J_JOYSTICK state
#define E_IN_MENU          3  
#define E_START_RECORD     4      
#define E_RECORDING        5
#define E_STOP_RECORD      6
#define E_START_PLAYBACK   7
#define E_PLAYING          8
#define E_STOP_PLAYBACK    9
#define E_REPORT_SLIDE     10 
#define E_REPORT_JOYSTICK  11
#define E_DISPLAY_JOYSTICK 12
#define E_SAVE             13

char *stateNames[] = {"Null", "Idle", "Running","In Menu", "Start Record",
		      "Recording", "Stop Record", "Start Playback",
		      "Playing", "Stop Playback", "Report Slide", 
		      "Report Joystick", "Display Joystick"};

// icon settings
#define DISP_JOYSTICK      0
#define DISP_RUN           1
#define DISP_RECORD        2
#define DISPLAY_PLAYBACK   3
#define DISPLAY_SAVE       4


