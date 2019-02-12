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

#define RIGHTANGLETURN 800 //780 //780 // 770 //850

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
         motor_move(PORT_FLYWHEEL, -1);
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
         while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) < autonCurrentFlywheelGoalRPM && !rapidFire)
         {
            delay(20);
         }
         if (!rapidFire)
         {
            delay(350);
         }
         motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
      }
      if (rapidFire)
      {
         setIndexerPower(-127);
         //rapid fire
         while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) > autonCurrentFlywheelGoalRPM - 16)
         {
            delay(20);
         }
         motor_move(PORT_FLYWHEEL, -3); //-3);
         autonCurrentFlywheelGoalRPM = MIDDLEFLAGRPM;
         autonCurrentFlywheelPower = MIDDLEFLAGPOWER;
         delay(100);
         motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
         delay(1000);
         motor_move(PORT_INDEXER, autonCurrentFlywheelPower + FRICTIONPOWER);

         rapidFire = false;
      }
      /*if (adi_digital_read(LIMITSWITCHPORT) == 1)
      {
         motor_move_relative(PORT_INDEXER, -400, 90);
         //motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower + EXTRAPOWER);
         delay(240);
         setIndexerPower(autonCurrentFlywheelPower + FRICTIONPOWER);
      }*/
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

void fullAutonFront(bool park, bool redAlliance)
{
   // Start the flywheel
   setFlywheelSpeed(FRONTTILEPOWER, FRONTTILERPM);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");

   //Pick up the ball from the cap and return
   forwardCoast(200, 40);
   forwardCoast(1850, 127);
   forwardCoast(200, 10);
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(350);
   backwardCoast(2600, 127);
   assignDriveMotorsPower(-80, -80);
   delay(300);
   assignDriveMotorsPower(-40, -40);
   delay(300);
   assignDriveMotorsPower(0, 0);

   setIndexerPower(-127);
   delay(190);
   setIndexerPower(HIGHFLAGPOWER);

   // Turn and shoot the balls
   forwardCoast(300, 50);
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(70);
   turnRight(RIGHTANGLETURN, 100, redAlliance);
   forwardCoast(400, 100);
   forwardCoast(200, 10);
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(200);
   assignDriveMotorsPower(0, 0);
   rapidFire = true;
   delay(1000);

   //Move forward and toggle the low flag
   turnRight(60, 200, redAlliance);
   forwardCoast(200, 60);
   forwardCoast(1400, 127);
   forwardCoast(550, 90);

   //Drive back and align with cap
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(500);
   backward(1130, 150);
   delay(150);
   turnLeft(RIGHTANGLETURN, 100, redAlliance);

   //Flip the cap with balls on it
   assignDriveMotorsPower(0, 0);
   task_suspend(flywheelTask);
   task_delete(flywheelTask);
   motor_move(PORT_FLYWHEEL, 0);

   setIndexerPower(-127);
   forwardCoast(1900, 60);
   setIndexerPower(-60);
   forward(530, 120);
   setIndexerPower(0);

   //Park if needed, otherwise move backwards
   if (park)
   {
      turnLeft(RIGHTANGLETURN + 25, 100, redAlliance);

      forwardCoast(4300, 127);
      assignDriveMotorsPower(-30, -30);
      delay(100);
      assignDriveMotorsPower(0, 0);
   }
   else
   {

      backward(300, 100);
      delay(1000);
   }
}

void flagAutonBack(bool park, bool redAlliance)
{
   // Start the flywheel
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   setFlywheelSpeed(BACKTILEPOWER, BACKTILERPM);

   // Shoot the ball
   delay(5000);
   setIndexerPower(-127);
   setFlywheelSpeed(BACKTILEPOWER + EXTRAPOWER, BACKTILERPM);
   delay(1500);
   setFlywheelSpeed(BACKTILEPOWER, BACKTILERPM);
   setIndexerPower(BACKTILEPOWER);

   // Align with the wall
   turnLeft(RIGHTANGLETURN - (RIGHTANGLETURN / 3), 100, redAlliance);
   assignDriveMotorsPower(-80, -80);
   delay(600);
   assignDriveMotorsPower(0, 0);

   // Pick up the ball
   forward(2900, 140);
   delay(300);
   backward(100, 140);
   setIndexerPower(0);

   // Turn off the flywheel
   task_suspend(flywheelTask);
   task_delete(flywheelTask);
   motor_move(PORT_FLYWHEEL, 0);

   // Park if needed
   if (park)
   {
      turnRight(RIGHTANGLETURN, 100, redAlliance);

      forwardCoast(2580, 127);
      assignDriveMotorsPower(-30, -30);
      delay(100);
      assignDriveMotorsPower(0, 0);
   }
}

void autonomous()
{
   task_t displayInfoTask = task_create(displayInfoAuton, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Info Task");

   switch (autonNumber)
   {
   case 1:
      flagAutonBack(true, redAlliance);
      break;
   case 2:
      flagAutonBack(false, redAlliance);
      break;
   case 3:
      fullAutonFront(true, redAlliance);
      break;
   case 4:
      fullAutonFront(false, redAlliance);
      break;
   }
}
