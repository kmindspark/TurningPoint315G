// All the motor definitions
#define PORT_DRIVERIGHTBACK 1
#define PORT_DRIVERIGHTFRONT 2
#define PORT_FLYWHEEL 3
#define PORT_DRIVELEFTBACK 5
#define PORT_DRIVELEFTCENTER 6
#define PORT_INDEXER 8
#define PORT_VISION 16
#define PORT_DRIVERIGHTCENTER 4
#define PORT_DRIVELEFTFRONT 7

#define HIGHFLAGPOWER 118 //125
#define HIGHFLAGRPM 102
#define MIDDLEFLAGPOWER 76 //77
#define MIDDLEFLAGRPM 62
#define BETWEENFLAGPOWER 99
#define BETWEEENFLAGRPM 83

#define FRONTTILEPOWER 106
#define FRONTTILERPM 92
#define BACKTILEPOWER 117
#define BACKTILERPM 97

#define LIMITSWITCHPORT 3

#define OVERRIDETEMP true
#define MAXALLOWEDTEMP 45

#define NUM_VISION_OBJECTS 2
#define VISIONTARGETSIG 3
#define REDFLAGSIG 2
#define BLUEFLAGSIG 1
#define FLAGPIXELHEIGHT 22
#define ANGLETOLERANCE 5
#define HEIGHTTOLERANCE 5

#define KPFLYWHEEL 0.5
#define KIFLYWHEEL 0.05

#define EXTRAPOWER 10
#define FRICTIONPOWER 0

#define DIFFBRAKE 5

#define KP 0.5
#define KI 0.01
#define INTEGRALLIMIT 4

extern int autonNumber;
extern bool redAlliance;
