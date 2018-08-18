#include "main.h"

#define PORT_DRIVELEFT 1
#define PORT_DRIVERIGHT 2
#define PORT_FLYWHEEL 3
#define PORT_INTAKE 4

#define HIGHFLAGPOWER 95
#define MIDDLEFLAGPOWER 60

#define OVERRIDETEMP false

void drive(void* param){
    while (true) {
        int left = controller_get_analog(CONTROLLER_MASTER, ANALOG_LEFT_Y);
        int right = controller_get_analog(CONTROLLER_MASTER, ANALOG_RIGHT_Y);
        // printf("%d, %d\n", left, right);
        motor_move(PORT_DRIVELEFT, left);
        motor_move(PORT_DRIVERIGHT, right);
        delay(20);
  }
}

void flywheel(void* param){
    while (true) {
        if (E_CONTROLLER_DIGITAL_X == 1){
            motor_move_velocity(PORT_FLYWHEEL, HIGHFLAGPOWER);
        }
        if (E_CONTROLLER_DIGITAL_A == 1){
            motor_move_velocity(PORT_FLYWHEEL, MIDDLEFLAGPOWER);
        }
        if (E_CONTROLLER_DIGITAL_B == 1 || (motor_get_temperature(PORT_FLYWHEEL) > 45 && !OVERRIDETEMP)){
            motor_move(PORT_FLYWHEEL, 0);
        }
    }
}

void intake(void* param){
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
    task_t driveTask = task_create(drive, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Drive Task");
    task_t flywheelTask = task_create(flywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
    task_t intakeTask = task_create(intake, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Intake Task");
}
