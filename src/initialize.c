#include "main.h"

int autonNumber = 0;
bool redAlliance = false;

#define NUMAUTONS 2

static const char *btnm_map[] = {"LF&HF&P F", "HF&P B", ""};
static const char *alliance_map[] = {"Red", "Blue", ""};

void initializeDriveMotors()
{
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

void initializeFlywheelMotor()
{
   motor_set_gearing(PORT_FLYWHEEL, E_MOTOR_GEARSET_36);
   motor_set_reversed(PORT_FLYWHEEL, true);
}

void initializeIntakeMotor()
{
   motor_set_gearing(PORT_INTAKE, E_MOTOR_GEARSET_18);
   motor_set_reversed(PORT_INTAKE, true);
}

static lv_res_t btnm_action(lv_obj_t *btnm, const char *txt)
{
   if (strcmp(txt, "Red") == 0)
   {
      redAlliance = true;
      lv_btnm_set_toggle(btnm, true, 1);
   }
   else if (strcmp(txt, "Blue") == 1)
   {
      redAlliance = false;
      lv_btnm_set_toggle(btnm, true, 2);
   }
   else
   {
      for (int i = 0; i < NUMAUTONS; i++)
      {
         if (strcmp(btnm_map[i], txt) == 0)
         {
            autonNumber = i + 1;
            break;
         }
      }
      lv_btnm_set_toggle(btnm, true, autonNumber);
   }

   return LV_RES_OK; /*Return OK because the button matrix is not deleted*/
}

void initialize()
{
   initializeDriveMotors();
   initializeFlywheelMotor();
   initializeIntakeMotor();
}

void disabled() {}
void competition_initialize()
{
   lv_theme_alien_init(40, NULL);

   lv_obj_t *title = lv_label_create(lv_scr_act(), NULL);
   lv_label_set_text(title, "Auton Selection");
   lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

   lv_obj_t *btnm = lv_btnm_create(lv_scr_act(), NULL);
   lv_btnm_set_map(btnm, btnm_map);
   lv_btnm_set_action(btnm, btnm_action);
   lv_obj_set_size(btnm, LV_HOR_RES - 40, LV_VER_RES / 3);
   lv_obj_align(btnm, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

   lv_obj_t *allianceM = lv_btnm_create(lv_scr_act(), NULL);
   lv_btnm_set_map(allianceM, alliance_map);
   lv_btnm_set_action(allianceM, btnm_action);
   lv_obj_set_size(allianceM, LV_HOR_RES - 40, 50);
   lv_obj_align(allianceM, btnm, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
}
