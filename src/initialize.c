#include "main.h"

void on_center_button() {
  static bool pressed = false;
  pressed = !pressed;
  if (pressed) {
    lcd_set_text(2, "I was pressed!");
  } else {
    lcd_clear_line(2);
  }
}

void initializeDriveMotors(){
    motor_set_gearing(PORT_DRIVELEFTFRONT, E_MOTOR_GEARSET_18);
    motor_set_gearing(PORT_DRIVERIGHTFRONT, E_MOTOR_GEARSET_18);
    motor_set_gearing(PORT_DRIVELEFTBACK, E_MOTOR_GEARSET_18);
    motor_set_gearing(PORT_DRIVERIGHTBACK, E_MOTOR_GEARSET_18);
    motor_set_reversed(PORT_DRIVELEFTFRONT, false);
    motor_set_reversed(PORT_DRIVERIGHTFRONT, true);
    motor_set_reversed(PORT_DRIVELEFTBACK, false);
    motor_set_reversed(PORT_DRIVERIGHTBACK, true);
}

void initializeFlywheelMotor(){
    motor_set_gearing(PORT_FLYWHEEL, E_MOTOR_GEARSET_36);
    motor_set_reversed(PORT_FLYWHEEL, true);
}

void initializeIntakeMotor(){
    motor_set_gearing(PORT_INTAKE, E_MOTOR_GEARSET_18);
    motor_set_reversed(PORT_INTAKE, false);
}

void initialize() {
  lcd_initialize();
  lcd_set_text(1, "Hello Beta Testers!");
  lcd_register_btn1_cb(on_center_button);

  initializeDriveMotors();
  intializeFlywheelMotor();
  initializeIntakeMotor();
}

// the following functions don't work presently because comp. control
// hasn't been fully implemented
void disabled() {}
void competition_initialize() {}
