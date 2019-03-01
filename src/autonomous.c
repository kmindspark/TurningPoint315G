#include "main.h"

void turnLeft(int ticks, int power, bool reversed);
void turnRight(int ticks, int power, bool reversed);

#define TURNENCODERPRECISION 4
#define MOVEENCODERPRECISION 7
#define NUMDRIVEMOTORS 6.0
#define VELOCITYLIMIT 1

int indexerDirectionAuton = 0;

int autonCurrentFlywheelPower = 0;
int autonCurrentFlywheelGoalRPM = 0;

bool rapidFire = false;
bool backRapidFire = false;
bool flywheelOff = false;

int currentBrakingTime = 110;

#define RIGHTANGLETURN 760 //780 //780 // 770 //850

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

   delay(100);

   while ((abs(motor_get_position(PORT_DRIVELEFTFRONT) - leftSide) + abs(motor_get_position(PORT_DRIVELEFTBACK) - leftSide) +
               (abs(motor_get_position(PORT_DRIVERIGHTFRONT) - rightSide) + abs(motor_get_position(PORT_DRIVERIGHTBACK) - rightSide)) +
               (abs(motor_get_position(PORT_DRIVELEFTCENTER - leftSide)) + abs(motor_get_position(PORT_DRIVERIGHTCENTER - rightSide))) >
           currentPrecision * NUMDRIVEMOTORS) /* ||
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

void oneSideLeft(int ticks, int power, bool reversed)
{
   if (reversed)
   {
      oneSideRight(ticks, power, false);
   }
   else
   {
      assignDriveMotorsDist(0, ticks, power, true, true);
   }
}

void oneSideRight(int ticks, int power, bool reversed)
{
   if (reversed)
   {
      oneSideLeft(ticks, power, false);
   }
   else
   {
      assignDriveMotorsDist(ticks, 0, power, true, true);
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
   if (indexerPower >= 0)
   {
      indexerDirectionAuton = 1;
   }
   else
   {
      indexerDirectionAuton = -1;
   }
}

void flywheelPID(void *param)
{
   double integral = 0;
   while (true)
   {
      printf("Here\n");
      // PID LOOP
      int difference = autonCurrentFlywheelGoalRPM - abs(motor_get_actual_velocity(PORT_FLYWHEEL));

      double proportional = KP * difference;
      integral = integral + KI * difference;

      if (integral > INTEGRALLIMIT)
      {
         integral = INTEGRALLIMIT;
      }
      else if (integral < -INTEGRALLIMIT)
      {
         integral = -INTEGRALLIMIT;
      }

      int currentAssignedFlywheelPower = autonCurrentFlywheelPower + (int)proportional + (int)integral;

      motor_move(PORT_FLYWHEEL, currentAssignedFlywheelPower);
      if (indexerDirectionAuton > -1)
      {
         setIndexerPower(currentAssignedFlywheelPower + FRICTIONPOWER);
      }

      delay(20);
   }
}

void setFlywheelSpeed(int goalPower, int goalRPM)
{
   autonCurrentFlywheelPower = goalPower;
   autonCurrentFlywheelGoalRPM = goalRPM;
}

void autonFlywheel(void *param)
{
   task_t flywheelPIDTask = task_create(flywheelPID, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel PID");

   while (true)
   {
      if (flywheelOff)
      {
         task_suspend(flywheelPIDTask);
         task_delete(flywheelPIDTask);
         motor_move(PORT_FLYWHEEL, 0);
      }
      if (rapidFire || backRapidFire)
      {
         task_suspend(flywheelPIDTask);
         setIndexerPower(-127);
         //rapid fire
         while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) > autonCurrentFlywheelGoalRPM - 13)
         {
         }

         if (backRapidFire)
         {
            setFlywheelSpeed(BACKTILEPOWER - 1, BACKTILERPM - 1);
            setIndexerPower(127);
            delay(460);
            setIndexerPower(-127);
         }
         else
         {
            motor_move(PORT_FLYWHEEL, -3); //-3);
            autonCurrentFlywheelGoalRPM = MIDDLEFLAGRPM;
            autonCurrentFlywheelPower = MIDDLEFLAGPOWER;
            delay(currentBrakingTime); //68
            motor_move(PORT_FLYWHEEL, autonCurrentFlywheelPower);
            delay(1000);
            motor_move(PORT_INDEXER, autonCurrentFlywheelPower + FRICTIONPOWER);
         }

         rapidFire = false;
         backRapidFire = false;
         task_resume(flywheelPIDTask);
      }
      delay(20);
   }
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
      sprintf(tempString2, "Current Flywheel RPM: %d", abs(motor_get_actual_velocity(PORT_FLYWHEEL)));
      sprintf(tempString4, "Battery Voltage: %d", autonNumber);
      sprintf(tempString3, "Flywheel Goal RPM: %d", autonCurrentFlywheelGoalRPM); //abs(motor_get_position(PORT_DRIVELEFTFRONT) - leftSide) + abs(motor_get_position(PORT_DRIVELEFTBACK) - leftSide) + (abs(motor_get_position(PORT_DRIVERIGHTFRONT) - rightSide) + abs(motor_get_position(PORT_DRIVERIGHTBACK) - rightSide)));
      // + abs(motor_get_position(PORT_DRIVERIGHTFRONT) - rightSide)
      lcd_set_text(1, tempString1);
      lcd_set_text(2, tempString2);
      lcd_set_text(3, tempString3);
      lcd_set_text(4, tempString4);

      delay(20);
   }
   return;
}

void fullAutonFront(bool park, bool redAlliance, bool twoflags, bool skills)
{
   // Start the flywheel
   setFlywheelSpeed(HIGHFLAGPOWER, HIGHFLAGRPM);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");

   //backwardCoast(1000, 100);
   //Pick up the ball from the cap and return
   forwardCoast(200, 40);
   forwardCoast(1850, 127);
   forwardCoast(200, 10);
   if (skills)
   {
      setIndexerPower(-100);
      forwardCoast(200, 127);
      setIndexerPower(1);
   }
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(75);
   backwardCoast(200, 60);
   backwardCoast(1850, 127);
   if (skills)
   {
      backwardCoast(200, 127);
   }
   backwardCoast(600, 80);
   assignDriveMotorsPower(-30, -30);
   delay(500);
   assignDriveMotorsPower(0, 0);

   setIndexerPower(-127);
   delay(170);
   setIndexerPower(FRONTTILEPOWER);

   // Turn and shoot the balls
   forwardCoast(270, 50); //315
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(58); //94/90 //75
   assignDriveMotorsPower(0, 0);
   delay(350);
   turnRight(RIGHTANGLETURN - 5, 70, redAlliance); //Used to be 100 power
   forwardCoast(70, 70);
   rapidFire = true;
   forwardCoast(1030, 70);
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(175);
   assignDriveMotorsPower(0, 0);

   //delay(1000);

   // Move forward and toggle the low flag
   /*
   turnRight(47, 200, redAlliance);
   forwardCoast(300, 127);
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(200);
   assignDriveMotorsPower(0, 0);
   turnLeft(47, 200, redAlliance);
   forwardCoast(600, 127);*/

   turnRight(42, 100, redAlliance);
   forwardCoast(900, 127);

   if (twoflags)
   {
      rapidFire = false;
      currentBrakingTime = 32;
      setFlywheelSpeed(HIGHFLAGPOWER - 8, HIGHFLAGRPM - 7);

      forwardCoast(800, 127);
      assignDriveMotorsPower(100, 100);
      delay(400);
      assignDriveMotorsPower(0, 0);
      backwardCoast(860, 100);
      assignDriveMotorsPower(10 + DIFFBRAKE, 10);
      delay(200);
      assignDriveMotorsPower(0, 0);
      turnLeft(RIGHTANGLETURN, 100, redAlliance);

      forwardCoast(320, 100);
      assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
      delay(200);
      assignDriveMotorsPower(0, 0);

      oneSideLeft(RIGHTANGLETURN + 50, 100, redAlliance);

      // Move forward, align, and fire
      setIndexerPower(-45);
      forwardCoast(250, 40);
      assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
      delay(100);
      assignDriveMotorsPower(0, 0);

      setIndexerPower(1);

      backwardCoast(370, 80);
      assignDriveMotorsPower(10 + DIFFBRAKE, 10);
      delay(200);
      assignDriveMotorsPower(0, 0);

      setIndexerPower(1);

      delay(1500);

      turnRight(375, 100, redAlliance);
      delay(50);
      rapidFire = true;

      // Turn off flywheel and flip cap and hit lower flag
      delay(800);
      task_suspend(flywheelTask);
      task_delete(flywheelTask);
      motor_move(PORT_FLYWHEEL, 0);
      setIndexerPower(-127);

      turnLeft(500, 80, redAlliance);
      setIndexerPower(-127);
      forwardCoast(950, 60);
      forwardCoast(690, 120);

      assignDriveMotorsPower(-100, -100);
      delay(400);
      assignDriveMotorsPower(0, 0);

      return;
   }

   forwardCoast(790, 90);
   // Drive back and align with cap
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(500);
   backward(1180, 150);
   delay(150);
   turnLeft(RIGHTANGLETURN + 30, 100, redAlliance);

   // Flip the cap with balls on it
   assignDriveMotorsPower(0, 0);
   flywheelOff = true;

   setIndexerPower(-127);
   forwardCoast(1900, 60);
   setIndexerPower(-10);
   forward(500, 120);
   setIndexerPower(-1);

   // Park if needed, otherwise move backwards
   if (park)
   {
      turnLeft(RIGHTANGLETURN + 20, 80, redAlliance);

      forwardCoast(4650, 127);
      assignDriveMotorsPower(-30, -30);
      delay(100);
      assignDriveMotorsPower(0, 0);

      delay(3000);

      turnRight(RIGHTANGLETURN, 100, redAlliance);
      assignDriveMotorsPower(70, 70);
      delay(300);
      assignDriveMotorsPower(0, 0);

      backward(400, 50);

      forwardCoast(3000, 127);
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

void flagAutonBack(bool park, bool redAlliance, bool troll)
{
   // Start the flywheel
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   //setFlywheelSpeed(BACKTILEPOWER, BACKTILERPM);

   setFlywheelSpeed(BACKTILEPOWER - 4, BACKTILERPM - 4);

   // Shoot the ball
   delay(1700);
   setIndexerPower(-127);
   delay(400);
   setIndexerPower(BACKTILEPOWER);

   setFlywheelSpeed(HIGHFLAGPOWER + 5, HIGHFLAGRPM + 5);

   // Align with the wall
   turnLeft(RIGHTANGLETURN - (RIGHTANGLETURN / 3) + 60, 100, redAlliance);
   assignDriveMotorsPower(-80, -80);
   delay(600);
   assignDriveMotorsPower(0, 0);

   // Pick up the ball

   //forward(3400, 150);
   forwardCoast(200, 40);
   forwardCoast(2850, 127);
   forwardCoast(200, 10);

   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(200);
   assignDriveMotorsPower(0, 0);

   if (troll)
   {
      turnLeft(RIGHTANGLETURN, 80, redAlliance);

      forwardCoast(325, 100);
      setIndexerPower(-50);
      forwardCoast(395, 40);
      setIndexerPower(-1);
      assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
      delay(200);
      assignDriveMotorsPower(0, 0);
      backwardCoast(400, 80);
      assignDriveMotorsPower(10 + DIFFBRAKE, 10);
      setIndexerPower(1);
      delay(200);
      assignDriveMotorsPower(0, 0);
      delay(500);

      turnLeft(2 * RIGHTANGLETURN - 400, 100, redAlliance);

      forwardCoast(539, 127);
      assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
      delay(200);
      assignDriveMotorsPower(0, 0);
      delay(100);

      turnLeft(520, 100, redAlliance);

      setIndexerPower(-127);
      delay(150);
      setIndexerPower(0);

      setIndexerPower(-1);
      assignDriveMotorsPower(85, 85);
      delay(400);
      assignDriveMotorsPower(0, 0);

      backwardCoast(560, 50);
      setIndexerPower(1);
      assignDriveMotorsPower(10 + DIFFBRAKE, 10);
      delay(58);
      assignDriveMotorsPower(0, 0);

      turnLeft(335, 80, redAlliance);

      backRapidFire = true;
      delay(1000);

      turnRight(280, 80, redAlliance);

      forwardCoast(715, 127);
   }
   else
   {
      backward(100, 140);

      turnRight(RIGHTANGLETURN, 100, redAlliance);
   }

   // Turn off the flywheel
   flywheelOff = true;

   if (park)
   {
      forwardCoast(2380, 127);
      assignDriveMotorsPower(-30, -30);
      delay(100);
      assignDriveMotorsPower(0, 0);
   }
}

void autonomous()
{
   task_t displayInfoTask = task_create(displayInfoAuton, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Info Task");

   /*forwardCoast(315, 50);
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(75);
   assignDriveMotorsPower(0, 0);
   delay(350);
   turnRight(RIGHTANGLETURN, 80, redAlliance); //Used to be 100 power
   return;*/

   switch (autonNumber)
   {
   case 1:
      flagAutonBack(true, redAlliance, false);
      break;
   case 2:
      flagAutonBack(false, redAlliance, false);
      break;
   case 3:
      flagAutonBack(true, redAlliance, true);
      break;
   case 4:
      flagAutonFront(true, redAlliance, false, true);
      break;
   case 5:
      fullAutonFront(true, redAlliance, false, false);
      break;
   case 6:
      fullAutonFront(false, redAlliance, false, false);
      break;
   case 7:
      fullAutonFront(false, redAlliance, true, false);
      break;
   }

   /*forwardCoast(315, 50);
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(75);
   assignDriveMotorsPower(0, 0);
   delay(350);
   turnRight(RIGHTANGLETURN, 80, false); //Used to be 100 power*/

   /*forwardCoast(800, 80);
   setIndexerPower(-50);
   forwardCoast(200, 50);
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(200);
   assignDriveMotorsPower(0, 0);

   setIndexerPower(1);

   backwardCoast(400, 80);
   assignDriveMotorsPower(10 + DIFFBRAKE, 10);
   delay(200);
   assignDriveMotorsPower(0, 0);*/

   //return;
}