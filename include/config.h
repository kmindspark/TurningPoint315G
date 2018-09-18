// All the motor definitions
#define PORT_DRIVELEFTFRONT 1
#define PORT_DRIVERIGHTFRONT 2
#define PORT_FLYWHEEL 3
#define PORT_INTAKE 4
#define PORT_DRIVELEFTBACK 5
#define PORT_DRIVERIGHTBACK 6
#define PORT_CAPLIFT 7
#define PORT_DRIVECENTER 8
#define PORT_VISION 9

#define HIGHFLAGPOWER 120
#define HIGHFLAGRPM 104
#define MIDDLEFLAGPOWER 78
#define MIDDLEFLAGRPM 68
#define BETWEENFLAGPOWER 100
#define BETWEEENFLAGRPM 90

#define FRONTTILEPOWER 110
#define FRONTTILERPM 97

#define BACKTILEPOWER 110
#define BACKTILERPM 97

#define OVERRIDETEMP true
#define MAXALLOWEDTEMP 45

#define NUM_VISION_OBJECTS 2
#define REDFLAGSIG 2
#define BLUEFLAGSIG 3
#define MIDDLEFLAGPIXELHEIGHT 46

#define KPFLYWHEEL 0.5
#define KIFLYWHEEL 0.05

extern int autonNumber;
extern bool redAlliance;