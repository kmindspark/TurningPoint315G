#include "main.h"

#define HIGHFLAGPOWER 95
#define MIDDLEFLAGPOWER 60

#define OVERRIDETEMP false
#define MAXALLOWEDTEMP 45

void drive(void* param){
    while (true) {
        int forward = controller_get_analog(CONTROLLER_MASTER, ANALOG_LEFT_Y);
        int turn = controller_get_analog(CONTROLLER_MASTER, ANALOG_RIGHT_X);
        // printf("%d, %d\n", left, right);
        motor_move(PORT_DRIVELEFT, forward + turn);
        motor_move(PORT_DRIVERIGHT, forward - turn);
        delay(20);
  }
}

void flywheel(void* param){
    while (true) {
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_X)){
            motor_move_velocity(PORT_FLYWHEEL, HIGHFLAGPOWER);
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_A)){
            motor_move_velocity(PORT_FLYWHEEL, MIDDLEFLAGPOWER);
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_B) || 
            (motor_get_temperature(PORT_FLYWHEEL) > MAXALLOWEDTEMP && !OVERRIDETEMP)){
            motor_move(PORT_FLYWHEEL, 0);
        }
    }
}

void intake(void* param){
    while (true) {
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_L1)){
            if (motor_get_power(PORT_INTAKE) != 127){
                motor_move(PORT_INTAKE, 127);
            }
            else {
                motor_move(PORT_INTAKE, 0);
            }
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_L2)){
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
    task_t driveTask = task_create(drive, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Drive Task");
    task_t flywheelTask = task_create(flywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
    task_t intakeTask = task_create(intake, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Intake Task");
}
