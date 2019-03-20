// All the motor definitions
#define PORT_DRIVERIGHTBACK 17
#define PORT_DRIVERIGHTFRONT 14
#define PORT_FLYWHEEL 12
#define PORT_DRIVELEFTBACK 1
#define PORT_DRIVELEFTCENTER 11
#define PORT_INDEXER 18
#define PORT_VISION 16
#define PORT_DRIVERIGHTCENTER 15
#define PORT_DRIVELEFTFRONT 10

#define HIGHFLAGPOWER 90 //125
#define HIGHFLAGRPM 390
#define MIDDLEFLAGPOWER 80 //77
#define MIDDLEFLAGRPM 400
#define BETWEENFLAGPOWER 99
#define BETWEEENFLAGRPM 530

#define FRONTTILEPOWER 106
#define FRONTTILERPM 552
#define BACKTILEPOWER 119
#define BACKTILERPM 594

#define LIMITSWITCHPORT 3

#define NUM_VISION_OBJECTS 2
#define VISIONTARGETSIG 3
#define REDFLAGSIG 2
#define BLUEFLAGSIG 1
#define FLAGPIXELHEIGHT 22
#define ANGLETOLERANCE 5
#define HEIGHTTOLERANCE 5

#define EXTRAPOWER 10
#define FRICTIONPOWER 0

#define DIFFBRAKE 6

#define KP 0.7          //0.7          //0.7          //0.7          //0.6
#define KI 0.005        //0.005        //0.005        //0.005        //0.005             //0.02
#define INTEGRALLIMIT 6 //4

#define DRIVEP 0.070  //t 0.085
#define DRIVEI 0.0075 //t 0.01 //01
#define DRIVED 0.083  //t 0.14 //0.69   //1.0
#define TDRIVEP 0.205 //0.085
#define TDRIVEI 0.013 //01
#define TDRIVED 0.08  //0.69   //1.0
#define DRIVEINTEGRALLIMIT 1000
#define DRIVEMAXVEL 1
#define DRIVEPOSTOL 15

#define SCRAPER_HOOD_POS -200
#define SCRAPER_DOWN_POS -570

extern int autonNumber;
extern bool redAlliance;