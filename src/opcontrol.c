#include "main.h"

#define HIGHFLAGPOWER 127
#define MIDDLEFLAGPOWER 80

#define OVERRIDETEMP true
#define MAXALLOWEDTEMP 45

int intakeDirection = 0;
int currentFlywheelPower = 0;

void drive(void* param){
    while (true) {
        int forward = controller_get_analog(CONTROLLER_MASTER, ANALOG_LEFT_Y);
        int turn = controller_get_analog(CONTROLLER_MASTER, ANALOG_RIGHT_X);
        /*motor_move(PORT_DRIVELEFTFRONT, forward + turn);
        motor_move(PORT_DRIVERIGHTFRONT, forward - turn);
        motor_move(PORT_DRIVELEFTBACK, forward + turn);
        motor_move(PORT_DRIVERIGHTBACK, forward - turn);*/

        adi_motor_set(PORT_DRIVELEFTFRONT, forward + turn);
        adi_motor_set(PORT_DRIVERIGHTFRONT, forward - turn);
        adi_motor_set(PORT_DRIVELEFTBACK, forward + turn);
        adi_motor_set(PORT_DRIVERIGHTBACK, forward - turn);

        delay(20);
  }
}

void flywheel(void* param){
    while (true) {
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_X)){
            currentFlywheelPower = HIGHFLAGPOWER;
            //motor_move_velocity(PORT_FLYWHEEL, currentFlywheelPower);
            motor_move(PORT_FLYWHEEL, 127);
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_A)){
            motor_move(PORT_FLYWHEEL, -10);
            delay(500);
            currentFlywheelPower = MIDDLEFLAGPOWER;
            motor_move(PORT_FLYWHEEL, currentFlywheelPower);
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_DOWN)){
            if (currentFlywheelPower > 0){
                currentFlywheelPower--;
                motor_move(PORT_FLYWHEEL, currentFlywheelPower);
                delay(100);
            }
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_UP)){
            if (currentFlywheelPower < 100){
                currentFlywheelPower++;
                motor_move(PORT_FLYWHEEL, currentFlywheelPower);
                delay(100);
            }   
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_B) || 
            (motor_get_temperature(PORT_FLYWHEEL) > MAXALLOWEDTEMP && !OVERRIDETEMP)){
            currentFlywheelPower = 0;
            motor_move(PORT_FLYWHEEL, currentFlywheelPower);
        }
    }
}

void intake(void* param){
    while (true) {
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_L1)){
            if (intakeDirection != 1){
                motor_move(PORT_INTAKE, 127);
                intakeDirection = 1;
            }
            else {
                motor_move(PORT_INTAKE, 0);
                intakeDirection = 0;
            }
            delay(200);
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_L2)){
            if (intakeDirection != -1){
                motor_move(PORT_INTAKE, -127);
                intakeDirection = -1;
            }
            else {
                motor_move(PORT_INTAKE, 0);
                intakeDirection = 0;
            }
            delay(200);
        }
    }
}

void displayInfo(void* param){
    while (true) {
        lv_obj_t * info = lv_label_create(lv_scr_act(), NULL);
        char tempString[100];
        sprintf(tempString, "Flywheel Temperature: %f", motor_get_temperature(PORT_FLYWHEEL));
        lv_label_set_text(info, "TEMP");
        delay(500);
    }
}

void opcontrol() {
    task_t driveTask = task_create(drive, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Drive Task");
    task_t flywheelTask = task_create(flywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
    task_t intakeTask = task_create(intake, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Intake Task");
    task_t displayInfoTask = task_create(displayInfo, "PROS", TASK_PRIORITY_MIN, TASK_STACK_DEPTH_DEFAULT, "Display Task");
}
