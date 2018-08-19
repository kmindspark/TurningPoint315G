#include "main.h"

#define HIGHFLAGPOWER 100
#define MIDDLEFLAGPOWER 80

#define OVERRIDETEMP true
#define MAXALLOWEDTEMP 45

int intakeDirection = 0;
int currentFlywheelRPM = 0;

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
            currentFlywheelRPM = HIGHFLAGPOWER;
            //motor_move_velocity(PORT_FLYWHEEL, currentFlywheelRPM);
            motor_move(PORT_FLYWHEEL, 127);
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_A)){
            motor_move_velocity(PORT_FLYWHEEL, -10);
            delay(500);
            currentFlywheelRPM = MIDDLEFLAGPOWER;
            motor_move_velocity(PORT_FLYWHEEL, currentFlywheelRPM);
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_DOWN)){
            if (currentFlywheelRPM > 0){
                currentFlywheelRPM--;
                motor_move_velocity(PORT_FLYWHEEL, currentFlywheelRPM);
                delay(100);
            }
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_UP)){
            if (currentFlywheelRPM < 100){
                currentFlywheelRPM++;
                motor_move_velocity(PORT_FLYWHEEL, currentFlywheelRPM);
                delay(100);
            }   
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_B) || 
            (motor_get_temperature(PORT_FLYWHEEL) > MAXALLOWEDTEMP && !OVERRIDETEMP)){
            currentFlywheelRPM = 0;
            motor_move(PORT_FLYWHEEL, 0);
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

char* genString(char* infoString, float infoVal){
    char buffer[100];
    sprintf(buffer,  "%s: %v", infoString, infoVal);
    return buffer;
}

void displayInfo(void* param){
    lv_theme_t * th = lv_theme_zen_init(210, &lv_font_symbol_20);
    lv_theme_set_current(th);
    
    while (true) {
        lv_obj_t * info = lv_label_create(lv_scr_act, NULL);
        char* tempString = genString("Flywheel Temperature", motor_get_temperature(PORT_FLYWHEEL));
        lv_label_set_text(info, tempString);
        delay(500);
    }
}

void opcontrol() {
    task_t driveTask = task_create(drive, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Drive Task");
    task_t flywheelTask = task_create(flywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
    task_t intakeTask = task_create(intake, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Intake Task");
    task_t displayInfoTask = task_create(displayInfo, "PROS", TASK_PRIORITY_MIN, TASK_STACK_DEPTH_DEFAULT, "Display Task");
}
