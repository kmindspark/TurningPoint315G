#include "main.h"
#include "config.h"

int autonNumber = 0;
bool redAlliance = false;

/*
void on_center_button() {
  static bool pressed = false;
  pressed = !pressed;
  if (pressed) {
    lcd_set_text(2, "I was pressed!");
  } else {
    lcd_clear_line(2);
  }
}*/

void initializeDriveMotors(){
    motor_set_gearing(PORT_DRIVELEFTFRONT, E_MOTOR_GEARSET_18);
    motor_set_gearing(PORT_DRIVERIGHTFRONT, E_MOTOR_GEARSET_18);
    motor_set_gearing(PORT_DRIVELEFTBACK, E_MOTOR_GEARSET_18);
    motor_set_gearing(PORT_DRIVERIGHTBACK, E_MOTOR_GEARSET_18);
    motor_set_gearing(PORT_DRIVECENTER, E_MOTOR_GEARSET_18);
    motor_set_reversed(PORT_DRIVELEFTFRONT, false);
    motor_set_reversed(PORT_DRIVERIGHTFRONT, true);
    motor_set_reversed(PORT_DRIVELEFTBACK, false);
    motor_set_reversed(PORT_DRIVERIGHTBACK, true);
    motor_set_reversed(PORT_DRIVECENTER, false);
}

void initializeFlywheelMotor(){
    motor_set_gearing(PORT_FLYWHEEL, E_MOTOR_GEARSET_36);
    motor_set_reversed(PORT_FLYWHEEL, true);
}

void initializeIntakeMotor(){
    motor_set_gearing(PORT_INTAKE, E_MOTOR_GEARSET_18);
    motor_set_reversed(PORT_INTAKE, true);
}

static lv_res_t setAuton1(lv_obj_t * btn){
    autonNumber = 1;
    return LV_RES_OK;
}

static lv_res_t setAuton2(lv_obj_t * btn){
    autonNumber = 2;
    return LV_RES_OK;
}

static lv_res_t setAuton3(lv_obj_t * btn){
    autonNumber = 3;
    return LV_RES_OK;
}

static lv_res_t setAuton4(lv_obj_t * btn){
    autonNumber = 4;
    return LV_RES_OK;
}

static lv_res_t redTeam(lv_obj_t * btn){
    redAlliance = true;
    return LV_RES_OK;
}

static lv_res_t blueTeam(lv_obj_t * btn){
    redAlliance = false;
    return LV_RES_OK;
}


void initialize() {
  /*lcd_initialize();
  lcd_set_text(1, "Hello Beta Testers!");
  lcd_register_btn1_cb(on_center_button);*/

  initializeDriveMotors();
  initializeFlywheelMotor();
  initializeIntakeMotor();

  lv_obj_t * title = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(title, "       Auton Selection       ");
  lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
  
  lv_obj_t * auton1 = lv_btn_create(lv_scr_act(), NULL);
  lv_btn_set_action(auton1, LV_BTN_ACTION_CLICK, setAuton1);
  lv_cont_set_fit(auton1, true, true);
  lv_obj_align(auton1, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
  lv_obj_t * auton1Label = lv_label_create(auton1, NULL);
  lv_label_set_text(auton1Label, "Shoot & Low");
  
  lv_obj_t * auton2 = lv_btn_create(lv_scr_act(), NULL);
  lv_btn_set_action(auton2, LV_BTN_ACTION_CLICK, setAuton2);
  lv_cont_set_fit(auton2, true, true);
  lv_obj_align(auton2, title, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);
  lv_obj_t * auton2Label = lv_label_create(auton2, NULL);
  lv_label_set_text(auton2Label, "Shoot, Low + P");
  
  lv_obj_t * redBtn = lv_btn_create(lv_scr_act(), NULL);
  lv_btn_set_action(redBtn, LV_BTN_ACTION_CLICK, redTeam);
  lv_cont_set_fit(redBtn, true, true);
  lv_obj_align(redBtn, auton1, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
  lv_obj_t * redLabel = lv_label_create(redBtn, NULL);
  lv_label_set_text(redLabel, "Red");
  
  lv_obj_t * blueBtn = lv_btn_create(lv_scr_act(), NULL);
  lv_btn_set_action(blueBtn, LV_BTN_ACTION_CLICK, blueTeam);
  lv_cont_set_fit(blueBtn, true, true);
  lv_obj_align(blueBtn, auton2, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
  lv_obj_t * blueLabel = lv_label_create(blueBtn, NULL);
  lv_label_set_text(blueLabel, "Blue");
}

void disabled() {}
void competition_initialize() {}
