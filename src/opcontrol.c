#include "main.h"

PORT_DRIVELEFT = 1
PORT_DRIVERIGHT = 2
PORT_FLYWHEEL = 3

HIGHFLAGPOWER = 95
MIDDLEFLAGPOWER = 60

void drive(){
    while (true) {
        int left = controller_get_analog(CONTROLLER_MASTER, ANALOG_LEFT_Y);
        int right = controller_get_analog(CONTROLLER_MASTER, ANALOG_RIGHT_Y);
        // printf("%d, %d\n", left, right);
        motor_move(PORT_DRIVELEFT, left);
        motor_move(PORT_DRIVERIGHT, right);
        delay(20);
  }
}

void flywheel(){
    while (true) {
        if (E_CONTROLLER_DIGITAL_X){
            motor_move_velocity(PORT_FLYWHEEL, HIGHFLAGPOWER)
        }
        if (E_CONTROLLER_DIGITAL_A){
            motor_move_velocity(PORT_FLYWHEEL, MIDDLEFLAGPOWER)
        }
        if (E_CONTROLLER_DIGITAL_B){
            motor_move(PORT_FLYWHEEL, 0)
        }
    }
}

void intake(){
    while (true) {
        
    }
}

void opcontrol() {
    task_t driveTask = task_create(drive, TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Drive Task")
    task_t flywheelTask = task_create(flywheel, TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task")
}
