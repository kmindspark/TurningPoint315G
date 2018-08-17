#include "main.h"

PORT_DRIVELEFT = 1
PORT_DRIVERIGHT = 2
PORT_FLYWHEEL = 3
PORT_INTAKE = 3

HIGHFLAGPOWER = 95
MIDDLEFLAGPOWER = 60

OVERRIDETEMP = 0

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
            motor_move_velocity(PORT_FLYWHEEL, HIGHFLAGPOWER);
        }
        if (E_CONTROLLER_DIGITAL_A){
            motor_move_velocity(PORT_FLYWHEEL, MIDDLEFLAGPOWER);
        }
        if (E_CONTROLLER_DIGITAL_B || (motor_get_temperature(PORT_FLYWHEEL) > 45 && OVERRIDETEMP == 0)){
            motor_move(PORT_FLYWHEEL, 0);
        }
    }
}

void intake(){
    while (true) {
        if (E_CONTROLLER_DIGITAL_L1){
            if (motor_get_power(PORT_INTAKE) != 127){
                motor_move(PORT_INTAKE, 127);
            }
            else {
                motor_move(PORT_INTAKE, 0);
            }
        }
        if (E_CONTROLLER_DIGITAL_L2){
            if (motor_get_power(PORT_INTAKE) != -127){
                motor_move(PORT_INTAKE, -127);
            }
            else {
                motor_move(PORT_INTAKE, 0);
            }
        }
    }
}

void opcontrol() {
    task_t driveTask = task_create(drive, TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Drive Task")
    task_t flywheelTask = task_create(flywheel, TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task")
    task_t intakeTask = task_create(intake, TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Intake Task")
}
