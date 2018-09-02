#include "main.h"
#include "config.h"
#include <stdio.h>
#include <inttypes.h>

#define GOALBATTERYVOLTAGE 7.5

void displayInfo(void* param){
    lcd_initialize();
    while (true){
        char tempString1[100];
        char tempString2[100];
        char tempString3[100];

        sprintf(tempString1, "Flywheel Current: %f", motor_get_current_draw(PORT_FLYWHEEL));
        sprintf(tempString2, "Current Flywheel RPM: %f", motor_get_actual_velocity(PORT_FLYWHEEL));
        sprintf(tempString3, "Battery Voltage: %d", battery_get_voltage());
        
        lcd_set_text(1, tempString1);
        lcd_set_text(2, tempString2);
        lcd_set_text(3, tempString3);

        controller_print(CONTROLLER_MASTER, 0, 0, "RPM: %.2f", motor_get_actual_velocity(PORT_FLYWHEEL));
        controller_print(CONTROLLER_MASTER, 1, 0, "Volts: %.2f", battery_get_voltage());

        delay(20);
    }
}

void opcontrol() {
    task_t displayInfoTask = task_create(displayInfo, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Info Task");

    motor_move(PORT_FLYWHEEL, 127);
    motor_move(PORT_INTAKE, 127);
    while (battery_get_voltage() > GOALBATTERYVOLTAGE){
        delay(1000);
    }

    motor_move(PORT_FLYWHEEL, 0);
    motor_move(PORT_INTAKE, 0);
    
    task_delete(displayInfoTask);

    controller_print(CONTROLLER_MASTER, 2, 0, "Complete.");
}
