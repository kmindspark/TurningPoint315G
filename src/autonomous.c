#include "main.h"

void turnLeft(int ticks, int power, bool reversed);
void turnRight(int ticks, int power, bool reversed);

#define ENCODERPRECISION 5
#define NUMDRIVEMOTORS 5

int autonCurrentFlywheelPower = 0;
int autonCurrentFlywheelGoalRPM = 0;

bool rapidFire = false;

#define RIGHTANGLETURN 850

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

void assignDriveMotorsDist(int leftSide, int rightSide, int power, bool clear)
{
   if (clear)
   {
      clearDriveMotors();
   }
   motor_move_absolute(PORT_DRIVELEFTFRONT, leftSide, power);
   motor_move_absolute(PORT_DRIVELEFTBACK, leftSide, power);
   motor_move_absolute(PORT_DRIVERIGHTFRONT, rightSide, power);
   motor_move_absolute(PORT_DRIVERIGHTBACK, rightSide, power);
   motor_move_absolute(PORT_DRIVECENTER, (leftSide + rightSide) / 2.0, power);
   while (abs(motor_get_position(PORT_DRIVELEFTFRONT) - leftSide) + abs(motor_get_position(PORT_DRIVELEFTBACK) - leftSide) +
              (abs(motor_get_position(PORT_DRIVERIGHTFRONT) - rightSide) + abs(motor_get_position(PORT_DRIVERIGHTBACK) - rightSide)) +
              (abs(motor_get_position(PORT_DRIVECENTER) - ((leftSide + rightSide) / 2))) >
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
   assignDriveMotorsDist(-ticks, ticks, power, true);
}

void turnRight(int ticks, int power, bool reversed)
{
   if (reversed)
   {
      turnLeft(-ticks, power, false);
   }
   assignDriveMotorsDist(ticks, -ticks, power, true);
}

void forwardCoast(int ticks, int power)
{
   clearDriveMotors();
   assignDriveMotorsPower(power, power);
   while (abs(motor_get_position(PORT_DRIVELEFTFRONT)) + abs(motor_get_position(PORT_DRIVELEFTBACK)) +
              abs(motor_get_position(PORT_DRIVERIGHTFRONT)) + abs(motor_get_position(PORT_DRIVERIGHTBACK)) +
              abs(motor_get_position(PORT_DRIVECENTER)) <
          ticks * NUMDRIVEMOTORS)
   {
      delay(20);
   }
   assignDriveMotorsPower(0, 0);
}

void backwardCoast(int ticks, int power)
{
   clearDriveMotors();
   assignDriveMotorsPower(-power, -power);
   while (abs(motor_get_position(PORT_DRIVELEFTFRONT)) + abs(motor_get_position(PORT_DRIVELEFTBACK)) +
              abs(motor_get_position(PORT_DRIVERIGHTFRONT)) + abs(motor_get_position(PORT_DRIVERIGHTBACK)) +
              abs(motor_get_position(PORT_DRIVECENTER)) <
          ticks * NUMDRIVEMOTORS)
   {
      delay(20);
   }
   assignDriveMotorsPower(0, 0);
}

void forward(int ticks, int power)
{
   assignDriveMotorsDist(ticks, ticks, power, true);
}

void forwardAbs(int ticks, int power)
{
   assignDriveMotorsDist(ticks, ticks, power, false);
}

void backward(int ticks, int power)
{
   assignDriveMotorsDist(-ticks, -ticks, power, true);
}

void backwardAbs(int ticks, int power)
{
   assignDriveMotorsDist(-ticks, -ticks, power, false);
}

void setIntakePower(int intakePower)
{
   motor_move(PORT_INTAKE, intakePower);
}

void setCapLiftPower(int capPower)
{
   motor_move(PORT_CAPLIFT, capPower);
}

void autonFlywheel(void *param)
{
   motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
   while (true)
   {
      if (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) > autonCurrentFlywheelGoalRPM + 15)
      {
         motor_move(PORT_FLYWHEEL, -15);
         while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) > autonCurrentFlywheelGoalRPM + 5)
         {
            delay(20);
         }
         motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
      }
      if (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) < autonCurrentFlywheelGoalRPM - 6)
      {
         motor_move(PORT_FLYWHEEL, 127);
         while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) < autonCurrentFlywheelGoalRPM)
         {
            delay(20);
         }
         delay(350);
         motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
      }
      if (rapidFire)
      {
         setIntakePower(127);
         motor_move(PORT_INTAKE, 127);
         //rapid fire
         while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) > autonCurrentFlywheelGoalRPM - 8)
         {
            delay(20);
         }

         motor_move(PORT_FLYWHEEL, -35);
         autonCurrentFlywheelGoalRPM = MIDDLEFLAGRPM;
         autonCurrentFlywheelPower = MIDDLEFLAGPOWER;
         delay(150);
         motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
         delay(1000);

         rapidFire = false;
      }
      delay(20);
   }
}

void setFlywheelSpeed(int goalPower, int goalRPM)
{
   autonCurrentFlywheelPower = goalPower;
   autonCurrentFlywheelGoalRPM = goalRPM;
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

void flagAutonFront(bool park, bool redAlliance)
{
   setCapLiftPower(-10);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   setFlywheelSpeed(FRONTTILEPOWER, FRONTTILERPM);

   //forwardCoast(2000, 127);
   forward(3000, 127);
   assignDriveMotorsPower(50, 50);
   delay(500);
   backwardAbs(400, 127);
   turnLeft(35, 127, redAlliance);

   while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) < FRONTTILERPM)
   {
      delay(20);
   }
   delay(500);
   setIntakePower(127);
   delay(500);
   turnRight(35, 127, redAlliance);

   if (park)
   {
      backward(1750, 127);
      turnLeft(RIGHTANGLETURN, 127, redAlliance);

      assignDriveMotorsPower(127, 127);
      delay(1750);
   }
   assignDriveMotorsPower(0, 0);
}

void flagAutonBack(bool park, bool redAlliance)
{
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   setFlywheelSpeed(BACKTILEPOWER, BACKTILERPM);

   while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) < BACKTILERPM)
   {
      delay(20);
   }
   delay(750);
   setIntakePower(127);
   delay(1000);

   if (park)
   {
      forward(2000, 127);
      turnLeft(RIGHTANGLETURN, 127, redAlliance);

      assignDriveMotorsPower(127, 127);
      delay(1750);
      assignDriveMotorsPower(0, 0);
   }
}

void fullAutonFront(bool park, bool redAlliance)
{
   setCapLiftPower(-10);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   setFlywheelSpeed(FRONTTILEPOWER, FRONTTILERPM);

   assignDriveMotorsPower(50, 50);
   delay(900);
   setIntakePower(-127);
   assignDriveMotorsPower(127, 127);
   delay(400);
   assignDriveMotorsPower(0, 0);
   backwardAbs(100, 127);

   turnLeft(RIGHTANGLETURN / 2 + 25, 127, redAlliance);

   forwardCoast(200, 70);
   forwardCoast(1200, 127);
   setIntakePower(127);
   forward(800, 127);
   delay(200);
   setIntakePower(0);

   assignDriveMotorsPower(-127, -127);
   delay(1200);
   assignDriveMotorsPower(0, 0);

   forward(150, 127);
   turnRight(RIGHTANGLETURN, 127, redAlliance);
   forward(200, 127);

   rapidFire = true;
   delay(1500);

   forward(1800, 127);

   if (park)
   {
      backwardCoast(3000, 127);
      backward(2500, 127);
      turnLeft(RIGHTANGLETURN, 127, redAlliance);
      assignDriveMotorsPower(127, 127);
      delay(1750);
      assignDriveMotorsPower(0, 0);
   }
   else
   {
      delay(1000);
      setIntakePower(0);
   }
}

void autonomous()
{
   task_t displayInfoTask = task_create(displayInfoAuton, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Info Task");
   autonNumber = 5;
   switch (autonNumber)
   {
   case 1:
      flagAutonFront(true, redAlliance);
      break;
   case 2:
      flagAutonFront(false, redAlliance);
      break;
   case 3:
      flagAutonBack(true, redAlliance);
      break;
   case 4:
      flagAutonBack(false, redAlliance);
      break;
   case 5:
      fullAutonFront(true, redAlliance);
      break;
   case 6:
      fullAutonFront(false, redAlliance);
      break;
   }
}