#include "main.h"
#include "config.h"

extern int autonNumber;
extern bool redAlliance;

void assignDriveMotors(int leftSide, int rightSide){
    motor_move(PORT_DRIVELEFTFRONT, leftSide);
    motor_move(PORT_DRIVELEFTBACK, leftSide);
    motor_move(PORT_DRIVERIGHTFRONT, rightSide);
    motor_move(PORT_DRIVERIGHTBACK, rightSide);
    motor_move(PORT_DRIVECENTER, (leftSide + rightSide)/2.0);
}

void clearDriveMotors(){
    motor_tare_position(PORT_DRIVELEFTFRONT);
    motor_tare_position(PORT_DRIVELEFTBACK);
    motor_tare_position(PORT_DRIVERIGHTFRONT);
    motor_tare_position(PORT_DRIVERIGHTBACK);
    motor_tare_position(PORT_DRIVECENTER);
}

void turnLeft(int ticks, bool reversed){

}

void turnRight(int ticks, bool reversed){

}

void flagAuton(bool park, bool redAlliance){

}

void autonomous(){
    switch(autonNumber){
        case 1:
            flagAuton(false, redAlliance);
        case 2:
            flagAuton(true, redAlliance);
    }
}
