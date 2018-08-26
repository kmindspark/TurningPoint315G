#include "main.h"
#include "config.h"
#include <stdio.h>
#include <inttypes.h>

#define HIGHFLAGPOWER 127
#define HIGHFLAGRPM 106
#define MIDDLEFLAGPOWER 82
#define MIDDLEFLAGRPM 66
#define BETWEENFLAGPOWER 115
#define BETWEEENFLAGRPM 90

#define OVERRIDETEMP true
#define MAXALLOWEDTEMP 45

#define NUM_VISION_OBJECTS 2
#define REDFLAGSIG 2
#define BLUEFLAGSIG 3
#define MIDDLEFLAGPIXELHEIGHT 46

int intakeDirection = 0;
int currentFlywheelPower = 0;
int currentFlywheelGoalRPM = 0;
int middleFlagXCoord = 0;

bool knownRPM = false;
bool redAlliance = false;

 #define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

void turnToFlag(int sigNum){
    vision_object_s_t sizeFlag = vision_get_by_sig(PORT_VISION, 0, sigNum);
    
    if (sizeFlag.x_middle_coord > VISION_FOV_WIDTH/2){
        adi_motor_set(PORT_DRIVELEFTFRONT, -40);
        adi_motor_set(PORT_DRIVERIGHTFRONT, 40);
        while (sizeFlag.x_middle_coord > VISION_FOV_WIDTH/2){
            printf("%d\n", (int) sizeFlag.x_middle_coord);
            sizeFlag = vision_get_by_sig(PORT_VISION, 0, sigNum);
            delay(20);
        }
        adi_motor_set(PORT_DRIVELEFTFRONT, 20);
        adi_motor_set(PORT_DRIVERIGHTFRONT, -20);
    }
    else {
        adi_motor_set(PORT_DRIVELEFTFRONT, 40);
        adi_motor_set(PORT_DRIVERIGHTFRONT, -40);
        while (sizeFlag.x_middle_coord < VISION_FOV_WIDTH/2){
            printf("%d\n", (int) sizeFlag.x_middle_coord);
            sizeFlag = vision_get_by_sig(PORT_VISION, 0, sigNum);
            delay(20);
        }
        adi_motor_set(PORT_DRIVELEFTFRONT, -20);
        adi_motor_set(PORT_DRIVERIGHTFRONT, 20);
    }

    delay(100);
    adi_motor_set(PORT_DRIVELEFTFRONT, 0);
    adi_motor_set(PORT_DRIVERIGHTFRONT, 0);
}

void moveDistToFlag(int sigNum){
    vision_object_s_t sizeFlag = vision_get_by_sig(PORT_VISION, 0, sigNum);
    
    if (sizeFlag.height < MIDDLEFLAGPIXELHEIGHT){
        adi_motor_set(PORT_DRIVELEFTFRONT, 40);
        adi_motor_set(PORT_DRIVERIGHTFRONT, 40);
        while (sizeFlag.height < MIDDLEFLAGPIXELHEIGHT){
            printf("%d\n", (int) sizeFlag.height);
            sizeFlag = vision_get_by_sig(PORT_VISION, 0, sigNum);
            delay(20);
        }
        adi_motor_set(PORT_DRIVELEFTFRONT, -20);
        adi_motor_set(PORT_DRIVERIGHTFRONT, -20);
        
    }
    else {
        adi_motor_set(PORT_DRIVELEFTFRONT, -40);
        adi_motor_set(PORT_DRIVERIGHTFRONT, -40);
        while (sizeFlag.height > MIDDLEFLAGPIXELHEIGHT){
            printf("%d\n", (int) sizeFlag.height);
            sizeFlag = vision_get_by_sig(PORT_VISION, 0, sigNum);
            delay(20);
        }
        adi_motor_set(PORT_DRIVELEFTFRONT, 20);
        adi_motor_set(PORT_DRIVERIGHTFRONT, 20);
    }

    delay(100);
    adi_motor_set(PORT_DRIVELEFTFRONT, 0);
    adi_motor_set(PORT_DRIVERIGHTFRONT, 0);
}

void drive(void* param){
    while (true) {
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_LEFT)){
            int sigNum = BLUEFLAGSIG;
            if (!redAlliance){
                sigNum = REDFLAGSIG;
            }
            turnToFlag(sigNum);
            moveDistToFlag(sigNum);
        }
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
            currentFlywheelGoalRPM = HIGHFLAGRPM;
            motor_move(PORT_FLYWHEEL, currentFlywheelPower);
            knownRPM = true;
        }
        else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_A)){
            currentFlywheelPower = MIDDLEFLAGPOWER;
            currentFlywheelGoalRPM = MIDDLEFLAGRPM;
            motor_move(PORT_FLYWHEEL, currentFlywheelPower);
            knownRPM = true;
        }
        else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_Y)){
            currentFlywheelPower = BETWEENFLAGPOWER;
            currentFlywheelGoalRPM = BETWEEENFLAGRPM;
            motor_move(PORT_FLYWHEEL, currentFlywheelPower);
            knownRPM = true;
        }
        else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_DOWN)){
            if (currentFlywheelPower > 0){
                currentFlywheelPower--;
                motor_move(PORT_FLYWHEEL, currentFlywheelPower);
                delay(300);
                knownRPM = false;
            }
        }
        else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_UP)){
            if (currentFlywheelPower < 127){
                currentFlywheelPower++;
                motor_move(PORT_FLYWHEEL, currentFlywheelPower);
                delay(300);
                knownRPM = false;
            }   
        }
        else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_B) || 
            (motor_get_temperature(PORT_FLYWHEEL) > MAXALLOWEDTEMP && !OVERRIDETEMP)){
            currentFlywheelPower = 0;
            motor_move(PORT_FLYWHEEL, currentFlywheelPower);
            knownRPM = false;
        }
        else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_RIGHT)){
            intakeDirection = 1;
            motor_move(PORT_INTAKE, 127);
            //rapid fire
            while (motor_get_actual_velocity(PORT_FLYWHEEL)*-1.0  > currentFlywheelGoalRPM - 4){
                if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_A) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_B) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_X) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_Y)){
                            break;
                }
            }
            motor_move(PORT_FLYWHEEL, -15);
            currentFlywheelGoalRPM = MIDDLEFLAGRPM;
            currentFlywheelPower = MIDDLEFLAGPOWER;
            while (motor_get_actual_velocity(PORT_FLYWHEEL)*-1.0 > currentFlywheelGoalRPM + 15){
                if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_A) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_B) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_X) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_Y)){
                            break;
                }
            }

            motor_move(PORT_FLYWHEEL, currentFlywheelPower);
        }
        else if (knownRPM) {
            if (motor_get_actual_velocity(PORT_FLYWHEEL)*-1.0 > currentFlywheelGoalRPM + 15){
                motor_move(PORT_FLYWHEEL, -15);
                while (motor_get_actual_velocity(PORT_FLYWHEEL)*-1.0 > currentFlywheelGoalRPM + 15){
                    if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_A) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_B) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_X) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_Y)){
                            break;
                    }
                }
                motor_move(PORT_FLYWHEEL, currentFlywheelPower);
            }
            if (motor_get_actual_velocity(PORT_FLYWHEEL)*-1.0  < currentFlywheelGoalRPM - 6){
                motor_move(PORT_FLYWHEEL, 127);
                while (motor_get_actual_velocity(PORT_FLYWHEEL)*-1.0  < currentFlywheelGoalRPM){
                    if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_A) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_B) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_X) ||
                        controller_get_digital(CONTROLLER_MASTER, DIGITAL_Y)){
                            break;
                    }
                }
                motor_move(PORT_FLYWHEEL, currentFlywheelPower);
            }
        }
        delay(20);
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
            delay(300);
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
            delay(300);
        }
        delay(20);
    }
}

void capLift(void* param){
    while (true){
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_R1)){
            motor_move(PORT_CAPLIFT, 127);
            while (controller_get_digital(CONTROLLER_MASTER, DIGITAL_R1)){

            }
            motor_move(PORT_CAPLIFT, 20);
        }
        if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_R2)){
            motor_move(PORT_CAPLIFT, -127);
            while (controller_get_digital(CONTROLLER_MASTER, DIGITAL_R2)){

            }
            motor_move(PORT_CAPLIFT, 0);
        }
        delay(20);
    }
}

void displayInfo(void* param){
    lcd_initialize();
    while (true){
        char tempString1[100];
        char tempString2[100];
        char tempString3[100];
        char tempString4[100];
        char tempString5[100];

        sprintf(tempString1, "Flywheel Temperature: %f", motor_get_temperature(PORT_FLYWHEEL));
        sprintf(tempString2, "Current Velocity: %f", motor_get_actual_velocity(PORT_FLYWHEEL));
        sprintf(tempString3, "Goal RPM: %d", currentFlywheelGoalRPM);
        sprintf(tempString4, "Goal Power: %d", currentFlywheelPower);
        sprintf(tempString5, "Middle Coord: %d", middleFlagXCoord);
        
        lcd_set_text(1, tempString1);
        lcd_set_text(2, tempString2);
        lcd_set_text(3, tempString3);
        lcd_set_text(4, tempString4);
        lcd_set_text(5, tempString5);
        delay(100);
    }
}

static lv_res_t redTeam(lv_obj_t * btn){
    redAlliance = true;
    return LV_RES_OK;
}

static lv_res_t blueTeam(lv_obj_t * btn){
    redAlliance = false;
    return LV_RES_OK;
}

void lvglInfo(){ //
    lv_obj_t * title = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(title, "The code is running.");
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);

    lv_obj_t * redBtn = lv_btn_create(lv_scr_act(), NULL);
    lv_btn_set_action(redBtn, LV_BTN_ACTION_CLICK, redTeam);
    lv_cont_set_fit(redBtn, true, true);
    lv_obj_align(redBtn, title, LV_ALIGN_OUT_BOTTOM_CENTER, 0, 10);
    lv_obj_t * redLabel = lv_label_create(redBtn, NULL);
    lv_label_set_text(redLabel, "Red");

    lv_obj_t * blueBtn = lv_btn_create(lv_scr_act(), NULL);
    lv_btn_set_action(blueBtn, LV_BTN_ACTION_CLICK, blueTeam);
    lv_cont_set_fit(blueBtn, true, true);
    lv_obj_align(blueBtn, redBtn, LV_ALIGN_OUT_BOTTOM_CENTER, 0, 10);
    lv_obj_t * blueLabel = lv_label_create(blueBtn, NULL);
    lv_label_set_text(blueLabel, "Blue");
}


void opcontrol() {
    task_t driveTask = task_create(drive, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Drive Task");
    task_t flywheelTask = task_create(flywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
    task_t intakeTask = task_create(intake, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Intake Task");
    task_t capLiftTask = task_create(capLift, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Cap Lift Task");
    
    lvglInfo();
}
