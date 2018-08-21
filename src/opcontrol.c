#include "main.h"

#define HIGHFLAGPOWER 127
#define MIDDLEFLAGPOWER 80
#define MIDDLEFLAGRPM 66

#define OVERRIDETEMP true
#define MAXALLOWEDTEMP 45

int intakeDirection = 0;
int currentFlywheelPower = 0;

 #define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

void drive(void* param){
    while (true) {
        int forward = controller_get_analog(CONTROLLER_MASTER, ANALOG_LEFT_Y);
        int turn = -1*controller_get_analog(CONTROLLER_MASTER, ANALOG_RIGHT_X);
        /*motor_move(PORT_DRIVELEFTFRONT, forward + turn);
        motor_move(PORT_DRIVERIGHTFRONT, forward - turn);
        motor_move(PORT_DRIVELEFTBACK, forward + turn);
        motor_move(PORT_DRIVERIGHTBACK, forward - turn);*/
        adi_motor_set(PORT_DRIVELEFTFRONT, max(-127, min(127, forward + turn)));
        adi_motor_set(PORT_DRIVERIGHTFRONT, max(-127, min(127, forward - turn)));
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
            while (motor_get_actual_velocity(PORT_FLYWHEEL) > MIDDLEFLAGRPM){

            }
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
            if (currentFlywheelPower < 127){
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
    /*while (true) {
        lv_obj_t * info = lv_label_create(lv_scr_act(), NULL);
        char tempString[100];
        sprintf(tempString, "Flywheel Temperature: %f", motor_get_temperature(PORT_FLYWHEEL));
        lv_label_set_text(info, "TEMP");
        delay(500);
    }*/
    while (true){
        lcd_initialize();
        char tempString[100];
        sprintf(tempString, "Flywheel Temperature: %f", motor_get_temperature(PORT_FLYWHEEL));
        lcd_set_text(1, tempString);
        delay(1000);
    }
}

void opcontrol() {
    task_t driveTask = task_create(drive, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Drive Task");
    task_t flywheelTask = task_create(flywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
    task_t intakeTask = task_create(intake, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Intake Task");
    task_t displayInfoTask = task_create(displayInfo, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Task");
}
