#include "main.h"

void turnLeft(int ticks, int power, bool reversed);
void turnRight(int ticks, int power, bool reversed);

#define ENCODERPRECISION 5
#define NUMDRIVEMOTORS 5

int autonCurrentFlywheelPower = 0;
int autonCurrentFlywheelGoalRPM = 0;

void clearDriveMotors()
{
   motor_tare_position(PORT_DRIVELEFTFRONT);
   motor_tare_position(PORT_DRIVELEFTBACK);
   motor_tare_position(PORT_DRIVERIGHTFRONT);
   motor_tare_position(PORT_DRIVERIGHTBACK);
   motor_tare_position(PORT_DRIVECENTER);
}

void assignDriveMotorsPower(int leftSide, int rightSide)
{
   motor_move(PORT_DRIVELEFTFRONT, leftSide);
   motor_move(PORT_DRIVELEFTBACK, leftSide);
   motor_move(PORT_DRIVERIGHTFRONT, rightSide);
   motor_move(PORT_DRIVERIGHTBACK, rightSide);
   motor_move(PORT_DRIVECENTER, (leftSide + rightSide) / 2.0);
}

void assignDriveMotorsDist(int leftSide, int rightSide, int centerWheel, int power)
{
   clearDriveMotors();
   motor_move_absolute(PORT_DRIVELEFTFRONT, leftSide, power);
   motor_move_absolute(PORT_DRIVELEFTBACK, leftSide, power);
   motor_move_absolute(PORT_DRIVERIGHTFRONT, rightSide, power);
   motor_move_absolute(PORT_DRIVERIGHTBACK, rightSide, power);
   motor_move_absolute(PORT_DRIVECENTER, centerWheel, power);
   while (abs(motor_get_position(PORT_DRIVELEFTFRONT) - leftSide) + abs(motor_get_position(PORT_DRIVELEFTBACK) - leftSide) +
              (abs(motor_get_position(PORT_DRIVERIGHTFRONT) - rightSide) + abs(motor_get_position(PORT_DRIVERIGHTBACK) - rightSide)) +
              (abs(motor_get_position(PORT_DRIVECENTER) - centerWheel)) >
          ENCODERPRECISION * NUMDRIVEMOTORS)
   {
      delay(20);
   }
   assignDriveMotorsPower(0, 0);
}

void turnLeft(int ticks, int power, bool reversed)
{
   if (reversed)
   {
      turnRight(-ticks, power, false);
   }
   assignDriveMotorsDist(-ticks, ticks, 0, power);
}

void turnRight(int ticks, int power, bool reversed)
{
   if (reversed)
   {
      turnLeft(-ticks, power, false);
   }
   assignDriveMotorsDist(ticks, -ticks, 0, power);
}

void forward(int ticks, int power)
{
   assignDriveMotorsDist(ticks, ticks, ticks, power);
}

void backward(int ticks, int power)
{
   assignDriveMotorsDist(-ticks, -ticks, -ticks, power);
}

void autonFlywheel(void *param)
{
   motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
   while (true)
   {
      if (motor_get_actual_velocity(PORT_FLYWHEEL) * -1.0 > autonCurrentFlywheelGoalRPM + 15)
      {
         motor_move(PORT_FLYWHEEL, -15);
         while (motor_get_actual_velocity(PORT_FLYWHEEL) * -1.0 > autonCurrentFlywheelGoalRPM + 5)
         {
            delay(20);
         }
         motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
      }
      if (motor_get_actual_velocity(PORT_FLYWHEEL) * -1.0 < autonCurrentFlywheelGoalRPM - 6)
      {
         motor_move(PORT_FLYWHEEL, 127);
         while (motor_get_actual_velocity(PORT_FLYWHEEL) * -1.0 < autonCurrentFlywheelGoalRPM)
         {
            delay(20);
         }
         motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
      }

      delay(20);
   }
}

void setFlywheelSpeed(int goalPower, int goalRPM)
{
   autonCurrentFlywheelPower = goalPower;
   autonCurrentFlywheelGoalRPM = goalRPM;
}

void setIntakePower(int intakePower)
{
   motor_move(PORT_INTAKE, intakePower);
}

void flagAuton(bool park, bool redAlliance)
{
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   setFlywheelSpeed(FRONTTILEPOWER, FRONTTILERPM);

   forward(3500, 127);
   backward(3500, 127);

   while (motor_get_actual_velocity(PORT_FLYWHEEL) * -1.0 < FRONTTILERPM)
   {
      delay(20);
   }
   delay(1000);
   setIntakePower(127);
   delay(1000);

   backward(2000, 127);
   turnLeft(850, 127, redAlliance);

   assignDriveMotorsPower(127, 127);
   delay(1750);
   assignDriveMotorsPower(0, 0);
}

void displayInfoAuton(void *param)
{
   lcd_initialize();
   while (true)
   {
      char tempString1[100];
      char tempString2[100];
      char tempString3[100];

      sprintf(tempString1, "Flywheel Temperature: %d", (int)motor_get_temperature(PORT_FLYWHEEL));
      sprintf(tempString2, "Current Flywheel RPM: %f", -1 * motor_get_actual_velocity(PORT_FLYWHEEL));
      sprintf(tempString3, "Battery Voltage: %d", battery_get_voltage());

      lcd_set_text(1, tempString1);
      lcd_set_text(2, tempString2);
      lcd_set_text(3, tempString3);

      delay(20);
   }
}

void autonomous()
{
   task_t displayInfoTask = task_create(displayInfoAuton, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Info Task");
   switch (autonNumber)
   {
   case 1:
      flagAuton(false, redAlliance);
   }
}