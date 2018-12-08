#include "main.h"

void turnLeft(int ticks, int power, bool reversed);
void turnRight(int ticks, int power, bool reversed);

#define TURNENCODERPRECISION 5
#define MOVEENCODERPRECISION 7
#define NUMDRIVEMOTORS 6.0
#define VELOCITYLIMIT 3

int indexerDirectionAuton = 0;

int autonCurrentFlywheelPower = 0;
int autonCurrentFlywheelGoalRPM = 0;

bool rapidFire = false;

#define RIGHTANGLETURN 780 //780 // 770 //850

void clearDriveMotors()
{
   motor_tare_position(PORT_DRIVELEFTFRONT);
   motor_tare_position(PORT_DRIVELEFTBACK);
   motor_tare_position(PORT_DRIVELEFTCENTER);
   motor_tare_position(PORT_DRIVERIGHTFRONT);
   motor_tare_position(PORT_DRIVERIGHTBACK);
   motor_tare_position(PORT_DRIVERIGHTCENTER);
}

void assignDriveMotorsPower(int leftSide, int rightSide)
{
   motor_move(PORT_DRIVELEFTFRONT, leftSide);
   motor_move(PORT_DRIVELEFTBACK, leftSide);
   motor_move(PORT_DRIVELEFTCENTER, leftSide);
   motor_move(PORT_DRIVERIGHTFRONT, rightSide);
   motor_move(PORT_DRIVERIGHTBACK, rightSide);
   motor_move(PORT_DRIVERIGHTCENTER, rightSide);
}

double averageVelocity()
{
   return (abs(motor_get_actual_velocity(PORT_DRIVELEFTFRONT)) + abs(motor_get_actual_velocity(PORT_DRIVELEFTBACK)) +
           abs(motor_get_actual_velocity(PORT_DRIVERIGHTFRONT)) + abs(motor_get_actual_velocity(PORT_DRIVERIGHTBACK))) /
          NUMDRIVEMOTORS;
}

void assignDriveMotorsDist(int leftSide, int rightSide, int power, bool clear, bool turn)
{
   if (clear)
   {
      clearDriveMotors();
   }

   int currentPrecision = MOVEENCODERPRECISION;
   if (turn)
   {
      currentPrecision = TURNENCODERPRECISION;
   }

   motor_move_absolute(PORT_DRIVELEFTFRONT, leftSide, power);
   motor_move_absolute(PORT_DRIVELEFTBACK, leftSide, power);
   motor_move_absolute(PORT_DRIVELEFTCENTER, leftSide, power);
   motor_move_absolute(PORT_DRIVERIGHTFRONT, rightSide, power);
   motor_move_absolute(PORT_DRIVERIGHTBACK, rightSide, power);
   motor_move_absolute(PORT_DRIVERIGHTCENTER, rightSide, power);

   while (abs(motor_get_position(PORT_DRIVELEFTFRONT) - leftSide) + abs(motor_get_position(PORT_DRIVELEFTBACK) - leftSide) +
              (abs(motor_get_position(PORT_DRIVERIGHTFRONT) - rightSide) + abs(motor_get_position(PORT_DRIVERIGHTBACK) - rightSide)) +
              (abs(motor_get_position(PORT_DRIVELEFTCENTER - leftSide)) + abs(motor_get_position(PORT_DRIVERIGHTCENTER - rightSide))) >
          currentPrecision * NUMDRIVEMOTORS /* ||
          (turn && (averageVelocity() >= VELOCITYLIMIT))*/
   )
   {
      delay(20);
   }

   delay(250);
   assignDriveMotorsPower(0, 0);
}

void turnLeft(int ticks, int power, bool reversed)
{
   if (reversed)
   {
      turnRight(ticks, power, false);
   }
   else
   {
      assignDriveMotorsDist(-ticks, ticks, power, true, true);
   }
}

void turnRight(int ticks, int power, bool reversed)
{
   if (reversed)
   {
      turnLeft(ticks, power, false);
   }
   else
   {
      assignDriveMotorsDist(ticks, -ticks, power, true, true);
   }
}

void forwardCoast(int ticks, int power)
{
   clearDriveMotors();
   assignDriveMotorsPower(power, power);
   while (abs(motor_get_position(PORT_DRIVELEFTFRONT)) + abs(motor_get_position(PORT_DRIVELEFTBACK)) +
              abs(motor_get_position(PORT_DRIVERIGHTFRONT)) + abs(motor_get_position(PORT_DRIVERIGHTBACK)) +
              abs(motor_get_position(PORT_DRIVELEFTCENTER)) + abs(motor_get_position(PORT_DRIVERIGHTCENTER)) <
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
              abs(motor_get_position(PORT_DRIVELEFTCENTER)) + abs(motor_get_position(PORT_DRIVERIGHTCENTER)) <
          ticks * NUMDRIVEMOTORS)
   {
      delay(20);
   }
   assignDriveMotorsPower(0, 0);
}

void forward(int ticks, int power)
{
   assignDriveMotorsDist(ticks, ticks, power, true, false);
}

void forwardAbs(int ticks, int power)
{
   assignDriveMotorsDist(ticks, ticks, power, false, false);
}

void backward(int ticks, int power)
{
   assignDriveMotorsDist(-ticks, -ticks, power, true, false);
}

void backwardAbs(int ticks, int power)
{
   assignDriveMotorsDist(ticks, ticks, power, false, false);
}

void setIndexerPower(int indexerPower)
{
   motor_move(PORT_INDEXER, indexerPower);
   if (indexerPower > 0)
   {
      indexerDirectionAuton = 1;
   }
   else
   {
      indexerDirectionAuton = 0;
   }
}

void autonFlywheel(void *param)
{
   motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
   motor_move(PORT_INDEXER, autonCurrentFlywheelPower + FRICTIONPOWER);
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
         setIndexerPower(100);
         while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) < autonCurrentFlywheelGoalRPM)
         {
            delay(20);
         }
         delay(350);
         motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
      }
      if (rapidFire)
      {
         setIndexerPower(-110);
         //rapid fire
         while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) > autonCurrentFlywheelGoalRPM - 8)
         {
            delay(20);
         }
         motor_move(PORT_FLYWHEEL, -120);
         autonCurrentFlywheelGoalRPM = MIDDLEFLAGRPM;
         autonCurrentFlywheelPower = MIDDLEFLAGPOWER;
         delay(200);
         motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
         delay(1000);
         motor_move(PORT_INDEXER, autonCurrentFlywheelPower + FRICTIONPOWER);

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
      char tempString4[100];

      int leftSide = -RIGHTANGLETURN;
      int rightSide = RIGHTANGLETURN;

      sprintf(tempString1, "Flywheel Temperature: %d", (int)motor_get_temperature(PORT_FLYWHEEL));
      sprintf(tempString2, "Current Flywheel RPM: %f", -1 * motor_get_actual_velocity(PORT_FLYWHEEL));
      sprintf(tempString3, "Battery Voltage: %d", autonNumber);
      sprintf(tempString4, "Turn: %d", abs(motor_get_position(PORT_DRIVELEFTFRONT) - leftSide) + abs(motor_get_position(PORT_DRIVELEFTBACK) - leftSide) + (abs(motor_get_position(PORT_DRIVERIGHTFRONT) - rightSide) + abs(motor_get_position(PORT_DRIVERIGHTBACK) - rightSide)));
      // + abs(motor_get_position(PORT_DRIVERIGHTFRONT) - rightSide)
      lcd_set_text(1, tempString1);
      lcd_set_text(2, tempString2);
      lcd_set_text(3, tempString3);
      lcd_set_text(4, tempString4);

      delay(20);
   }
   return;
}

void flagAutonFront(bool park, bool redAlliance)
{
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   setFlywheelSpeed(FRONTTILEPOWER, FRONTTILERPM);

   //forwardCoast(2000, 200);
   forward(3000, 200);
   assignDriveMotorsPower(50, 50);
   delay(500);
   backwardAbs(-200, 100);
   turnLeft(35, 200, redAlliance);

   while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) < FRONTTILERPM)
   {
      delay(20);
   }
   delay(500);
   setIndexerPower(-127);
   delay(500);
   turnRight(35, 200, redAlliance);

   if (park)
   {
      backward(2050, 150);
      turnLeft(RIGHTANGLETURN, 100, redAlliance);

      assignDriveMotorsPower(127, 200);
      delay(1750);
   }
   assignDriveMotorsPower(0, 0);

   task_suspend(flywheelTask);
   task_delete(flywheelTask);
   motor_move(PORT_FLYWHEEL, 0);
}

void flagAutonBackOld(bool park, bool redAlliance)
{
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   setFlywheelSpeed(BACKTILEPOWER, BACKTILERPM);

   while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) < BACKTILERPM)
   {
      delay(20);
   }
   delay(500);
   setIndexerPower(-127);
   delay(1500);

   turnRight(RIGHTANGLETURN / 3 - 20, 100, redAlliance);

   if (park)
   {
      forward(1400, 120);
      turnLeft(RIGHTANGLETURN, 100, redAlliance);

      assignDriveMotorsPower(127, 127);
      delay(1750);
      assignDriveMotorsPower(0, 0);
   }

   task_suspend(flywheelTask);
   task_delete(flywheelTask);
   motor_move(PORT_FLYWHEEL, 0);
}

void flagAutonBack(bool park, bool redAlliance)
{
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   setFlywheelSpeed(BACKTILEPOWER, BACKTILERPM);

   delay(4000);
   setIndexerPower(-127);
   delay(1000);

   turnLeft(RIGHTANGLETURN - (RIGHTANGLETURN / 3) + 40, 100, redAlliance);

   forward(2700, 140);

   delay(300);
   backward(400, 140);

   task_suspend(flywheelTask);
   task_delete(flywheelTask);
   motor_move(PORT_FLYWHEEL, 0);

   if (park)
   {
      turnRight(RIGHTANGLETURN, 100, redAlliance);

      forwardCoast(3080, 127);
      assignDriveMotorsPower(-30, -30);
      delay(100);
      assignDriveMotorsPower(0, 0);
   }
}

void fullAutonFrontOldPath(bool park, bool redAlliance)
{
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   setFlywheelSpeed(HIGHFLAGPOWER, HIGHFLAGRPM);

   forwardCoast(800, 127);
   forwardCoast(1000, 50);
   forwardCoast(600, 100);
   backwardCoast(200, 50);
   backward(2100, 190);

   turnLeft(RIGHTANGLETURN / 2 + 110, 120, redAlliance);

   forwardCoast(200, 70);
   forwardCoast(1800, 127);
   forward(400, 120);
   assignDriveMotorsPower(0, 0);

   turnLeft(60, 200, redAlliance);
   backwardCoast(3000, 127);
   assignDriveMotorsPower(-100, -100);
   delay(300);
   assignDriveMotorsPower(0, 0);

   forwardCoast(200, 50); //230, 150
   turnRight(RIGHTANGLETURN, 100, redAlliance);
   //backward(200, 50);

   /*while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) < HIGHFLAGRPM)
   {
      delay(20);
   }*/
   rapidFire = true;
   delay(500);

   turnRight(30, 200, redAlliance);

   /*forwardCoast(2800, 120); //3100
   assignDriveMotorsPower(90, 90);
   delay(300);*/
   forward(2400, 180); //2800

   task_suspend(flywheelTask);
   task_delete(flywheelTask);
   motor_move(PORT_FLYWHEEL, 0);

   if (park)
   {
      //backwardCoast(500, 127);
      backward(5100, 200);
      delay(150);
      turnLeft(RIGHTANGLETURN, 100, redAlliance);
      assignDriveMotorsPower(127, 127);
      delay(1750);
      assignDriveMotorsPower(0, 0);
   }
   else
   {
      delay(1000);
   }
}

void fullAutonFront(bool park, bool redAlliance)
{
   //setFlywheelSpeed(FRONTTILEPOWER, FRONTTILERPM);
   setFlywheelSpeed(HIGHFLAGPOWER, HIGHFLAGRPM);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");

   forwardCoast(200, 40);
   forwardCoast(2600, 110);
   //forward(600, 90);
   assignDriveMotorsPower(-30 - DIFFBRAKE, -30);
   //assignDriveMotorsPower(-30, -30);
   delay(150);
   assignDriveMotorsPower(-60 - DIFFBRAKE, -60);
   delay(100);
   assignDriveMotorsPower(-127, DIFFBRAKE - 127);
   delay(200);
   //turnLeft(60, 200, redAlliance);
   backwardCoast(2600, 127);
   assignDriveMotorsPower(-50, -50);
   delay(300);
   assignDriveMotorsPower(0, 0);

   forwardCoast(320, 50);                       //200, 230, 150
   turnRight(RIGHTANGLETURN, 100, redAlliance); //added 12 here
   forwardCoast(500, 100);
   assignDriveMotorsPower(-30 - DIFFBRAKE, -30);
   delay(100);
   assignDriveMotorsPower(0, 0);
   //backward(200, 50);

   /*while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) < HIGHFLAGRPM)
   {
      delay(20);
   }*/
   rapidFire = true;
   delay(1000);

   turnRight(60, 200, redAlliance); //80

   /*forwardCoast(2800, 120); //3100
   assignDriveMotorsPower(90, 90);
   delay(300);*/
   forwardCoast(200, 60); //2800
   forwardCoast(1400, 127);
   forwardCoast(200, 60);

   task_suspend(flywheelTask);
   task_delete(flywheelTask);
   motor_move(PORT_FLYWHEEL, 0);

   //backwardCoast(500, 127);
   delay(200);
   backward(1500, 150); //2000
   delay(150);
   turnLeft(RIGHTANGLETURN + 70, 100, redAlliance);
   //assignDriveMotorsPower(-127, -127);
   //delay(400);
   assignDriveMotorsPower(0, 0);
   setIndexerPower(-127);
   forwardCoast(1200, 60);
   forward(1030, 120);
   setIndexerPower(0);

   if (park)
   {
      turnLeft(RIGHTANGLETURN + 10, 100, redAlliance);

      /*assignDriveMotorsPower(127, 127);
      delay(2200);
      assignDriveMotorsPower(-30, -30);
      delay(200);*/

      forwardCoast(5500, 127);
      assignDriveMotorsPower(-30, -30);
      delay(100);
      assignDriveMotorsPower(0, 0);
   }
   else
   {
      delay(1000);
   }
}

void autonomous()
{
   task_t displayInfoTask = task_create(displayInfoAuton, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Info Task");

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
