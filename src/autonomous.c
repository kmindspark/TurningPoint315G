#include "main.h"

void turnLeft(int ticks, int power, bool reversed);
void turnRight(int ticks, int power, bool reversed);
void oneSideLeft(int ticks, int power, bool reversed);
void oneSideRight(int ticks, int power, bool reversed);

#define TURNENCODERPRECISION 5
#define MOVEENCODERPRECISION 7
#define NUMDRIVEMOTORS 6.0
#define VELOCITYLIMIT 1

int indexerInUseAuton = false;
bool scraperInUseAuton = false;

int autonCurrentFlywheelPower = 0;
int autonCurrentFlywheelGoalRPM = 0;

bool rapidFire = false;
bool backRapidFire = false;
bool flywheelOff = false;
bool rpmShot = false;

int currentBrakingTime = 110;

#define RIGHTANGLETURN 725 //780 //780 // 770 //850

#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

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
           (abs(motor_get_position(PORT_DRIVELEFTCENTER) - leftSide)) + abs(motor_get_position(PORT_DRIVERIGHTCENTER) - rightSide)) >
          currentPrecision * NUMDRIVEMOTORS) /* ||
          (turn && (averageVelocity() >= VELOCITYLIMIT))*/
   {
      delay(20);
   }

   delay(250);
   assignDriveMotorsPower(0, 0);
}

void assignDriveMotorsDistNew(int leftSide, int rightSide, int power, bool turn)
{
   clearDriveMotors();

   double curP = DRIVEP;
   double curI = DRIVEI;
   double curD = DRIVED;

   if (turn)
   {
      curP = TDRIVEP;
      curI = TDRIVEI;
      curD = TDRIVED;
   }

   int leftErrorChange = 0;
   int rightErrorChange = 0;
   int leftError = 3 * leftSide;
   int rightError = 3 * rightSide;
   int prevLeftError = 3 * leftSide;
   int prevRightError = 3 * rightSide;
   int leftIntegral = 0;
   int rightIntegral = 0;
   while (abs(leftErrorChange) + abs(rightErrorChange) > DRIVEMAXVEL || abs(leftError) + abs(rightError) > DRIVEPOSTOL)
   {
      leftIntegral += leftError;
      rightIntegral += rightError;

      if (leftIntegral > DRIVEINTEGRALLIMIT)
      {
         leftIntegral = DRIVEINTEGRALLIMIT;
      }
      if (leftIntegral < -DRIVEINTEGRALLIMIT)
      {
         leftIntegral = -DRIVEINTEGRALLIMIT;
      }
      if (rightIntegral > DRIVEINTEGRALLIMIT)
      {
         rightIntegral = DRIVEINTEGRALLIMIT;
      }
      if (rightIntegral < -DRIVEINTEGRALLIMIT)
      {
         rightIntegral = -DRIVEINTEGRALLIMIT;
      }

      leftErrorChange = leftError - prevLeftError;
      rightErrorChange = rightError - prevRightError;

      prevLeftError = leftError;
      prevRightError = rightError;

      int rightCorrection = rightError - leftError;
      double CORR = 0.8; //0.60;
      if (abs(rightError) < 1050 && abs(leftError) < 1050 && rightSide > 1050 && leftSide > 1050)
      {
         rightCorrection = rightCorrection - 30;
         //120; // - 70;
      }
      if (abs(rightError) < 550 && abs(leftError) < 550)
      {
         CORR = 0.0; //.1; //1; // 0.55;
      }
      if (turn)
      {
         CORR = 0;
      }
      /*if (rightError < 600 && leftError < 600 && leftSide > 0 && rightSide > 0)
      {
         /*assignDriveMotorsPower(-2, -2 - DIFFBRAKE);
         delay(300);
         assignDriveMotorsPower(0, 0);
         break;*/
      /*
         rightCorrection = DIFFBRAKE + 2;
      }*/

      assignDriveMotorsPower(min(power, max(-power, leftError * DRIVEP + leftIntegral * DRIVEI + leftErrorChange * DRIVED - 1.0 * rightCorrection * CORR)), min(power, max(-power, rightError * DRIVEP + rightIntegral * DRIVEI + rightErrorChange * DRIVED + rightCorrection * CORR)));

      delay(5);

      leftError = 3 * leftSide - motor_get_position(PORT_DRIVELEFTFRONT) - motor_get_position(PORT_DRIVELEFTCENTER) - motor_get_position(PORT_DRIVELEFTBACK);
      rightError = 3 * rightSide - motor_get_position(PORT_DRIVERIGHTFRONT) - motor_get_position(PORT_DRIVERIGHTCENTER) - motor_get_position(PORT_DRIVERIGHTBACK);
   }
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
      assignDriveMotorsDistNew(0, ticks, power, true);
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
      assignDriveMotorsDistNew(ticks, 0, power, true);
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
   assignDriveMotorsPower(30, 30 + DIFFBRAKE);
   delay(100); /*
   assignDriveMotorsPower(70, 70 + DIFFBRAKE);
   delay(100);
   assignDriveMotorsPower(100, 100 + DIFFBRAKE);
   delay(100);*/
   assignDriveMotorsDistNew(ticks, ticks, power, false);
}

void forwardAbs(int ticks, int power)
{
   assignDriveMotorsDistNew(ticks, ticks, power, false);
}

void backward(int ticks, int power)
{
   assignDriveMotorsPower(-30, -30 - DIFFBRAKE);
   delay(00);
   assignDriveMotorsPower(-70, -70 - DIFFBRAKE);
   delay(00);
   assignDriveMotorsPower(-100, -100 - DIFFBRAKE);
   delay(00);
   assignDriveMotorsDistNew(-ticks, -ticks, power, false);
}

void backwardAbs(int ticks, int power)
{
   assignDriveMotorsDistNew(ticks, ticks, power, false);
}

void setIndexerPower(int indexerPower)
{
   motor_move(PORT_INDEXER, indexerPower);
   if (indexerPower >= 0)
   {
      indexerInUseAuton = false;
   }
   else
   {
      indexerInUseAuton = true;
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

      if (!scraperInUseAuton)
      {
         motor_move(PORT_FLYWHEEL, currentAssignedFlywheelPower);
      }
      if (!indexerInUseAuton)
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
   armFlywheelKill();

   while (true)
   {
      if (flywheelOff)
      {
         task_suspend(flywheelPIDTask);
         task_delete(flywheelPIDTask);
         motor_move(PORT_FLYWHEEL, 0);
         rapidFire = false;
         backRapidFire = false;
         return;
      }
      if (rapidFire || backRapidFire)
      {
         indexerInUseAuton = true;
         setIndexerPower(-127);
         //rapid fire

         int prevVal = adi_digital_read(LIMITSWITCHPORT);
         while (prevVal == adi_digital_read(LIMITSWITCHPORT) || adi_digital_read(LIMITSWITCHPORT) == 1)
         {
            prevVal = adi_digital_read(LIMITSWITCHPORT);
            delay(5);
         }

         delay(50);

         if (!rpmShot && !backRapidFire)
         {
            scraperInUseAuton = true;
            motor_move(PORT_FLYWHEEL, -127);
            delay(800);
         }

         if (backRapidFire)
         {
            indexerInUseAuton = false;
            setIndexerPower(0);
            delay(000);
            indexerInUseAuton = true;
            setIndexerPower(-127);
            delay(300);
         }

         indexerInUseAuton = false;

         rapidFire = false;
      }
      delay(5);
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
      char tempString5[100];

      int leftSide = -RIGHTANGLETURN;
      int rightSide = RIGHTANGLETURN;

      sprintf(tempString1, "Flywheel Temperature: %d", (int)motor_get_temperature(PORT_FLYWHEEL));
      sprintf(tempString2, "Current Flywheel RPM: %d", abs(motor_get_actual_velocity(PORT_FLYWHEEL)));
      sprintf(tempString4, "Battery Voltage: %d", autonNumber);
      sprintf(tempString3, "Flywheel Goal RPM: %d", autonCurrentFlywheelGoalRPM); //abs(motor_get_position(PORT_DRIVELEFTFRONT) - leftSide) + abs(motor_get_position(PORT_DRIVELEFTBACK) - leftSide) + (abs(motor_get_position(PORT_DRIVERIGHTFRONT) - rightSide) + abs(motor_get_position(PORT_DRIVERIGHTBACK) - rightSide)));
      sprintf(tempString5, "Indexer Temperature: %d", (int)motor_get_temperature(PORT_INDEXER));
      // + abs(motor_get_position(PORT_DRIVERIGHTFRONT) - rightSide)
      lcd_set_text(1, tempString1);
      lcd_set_text(2, tempString2);
      lcd_set_text(3, tempString3);
      lcd_set_text(4, tempString4);
      lcd_set_text(5, tempString5);

      delay(20);
   }
   return;
}

void moveScraper(int pos)
{
   scraperInUseAuton = true;
   motor_move_relative(PORT_FLYWHEEL, pos, 200);
}

void resetScraper()
{
   scraperInUseAuton = false;
}

void armFlywheelKill()
{
   int32_t now = millis();
   while (millis() - now < 14900)
   {
      delay(10);
   }
   flywheelOff = true;
}

void fullAutonFront(bool park, bool redAlliance, bool twoflags, bool skills)
{
   // Start the flywheel
   setFlywheelSpeed(HIGHFLAGPOWER, HIGHFLAGRPM);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");

   forwardCoast(200, 40);
   forwardCoast(1850, 127);
   forwardCoast(200, 10);

   assignDriveMotorsPower(-10, -10 - DIFFBRAKE);
   delay(110);
   backwardCoast(200, 60);
   backwardCoast(1850, 127);

   backwardCoast(600, 80);
   assignDriveMotorsPower(-30, -30);
   delay(500);
   assignDriveMotorsPower(0, 0);

   setIndexerPower(-127);
   delay(120);
   setIndexerPower(FRONTTILEPOWER);

   // Turn and shoot the balls
   forward(130, 70);
   turnRight(RIGHTANGLETURN, 70, redAlliance); //Used to be 100 power
   forwardCoast(120, 70);
   rapidFire = true;
   delay(600);
   scraperInUseAuton = false;
   forwardCoast(900, 105);
   forwardCoast(180, 30);
   forwardCoast(100, 10);
   delay(150);

   setFlywheelSpeed(HIGHFLAGPOWER - 6, HIGHFLAGRPM - 36);

   turnRight(60, 100, redAlliance);
   forwardCoast(1100, 80);
   forwardCoast(200, 20);

   //forward(200, 105);

   if (twoflags)
   {
      rapidFire = false;
      currentBrakingTime = 10;

      delay(200);
      backwardCoast(100, 40);
      backward(2820, 100);
      assignDriveMotorsPower(10, 10 + DIFFBRAKE);
      delay(200);
      assignDriveMotorsPower(0, 0);
      delay(200);

      /*turnLeft(RIGHTANGLETURN, 70, redAlliance);
      assignDriveMotorsPower(-40, -40);
      delay(600);
      forward(500, 100);
*/
      turnLeft(315, 100, redAlliance);
      scraperInUseAuton = true;
      motor_move(PORT_FLYWHEEL, -127);
      delay(10);
      forward(960, 100);
      delay(70);
      motor_move(PORT_FLYWHEEL, -20);

      assignDriveMotorsPower(-40, -40);
      delay(350);
      assignDriveMotorsPower(0, 0);
      delay(300);

      motor_move_relative(PORT_FLYWHEEL, 80, 50);
      delay(200);
      backward(400, 127);
      motor_move(PORT_FLYWHEEL, -127);
      delay(430);

      assignDriveMotorsPower(15, 15 + DIFFBRAKE);
      delay(150);
      forwardCoast(150, 30);

      scraperInUseAuton = false;
      forwardCoast(250, 70);
      delay(500);

      rapidFire = true;
      delay(500);

      flywheelOff = true;

      return;
   }

   setIndexerPower(-127);

   forwardCoast(790, 90);
   // Drive back and align with cap
   assignDriveMotorsPower(-10 - DIFFBRAKE, -10);
   delay(150);
   assignDriveMotorsPower(0, 0);
   delay(300);
   backwardCoast(1300, 100);
   assignDriveMotorsPower(10 + DIFFBRAKE, 10);
   delay(75);
   assignDriveMotorsPower(0, 0);
   delay(200);
   turnLeft(RIGHTANGLETURN + 50, 100, redAlliance);

   // Flip the cap with balls on it
   assignDriveMotorsPower(0, 0);
   flywheelOff = true;

   forwardCoast(1400, 60);
   setIndexerPower(-10);
   forward(1000, 120);
   setIndexerPower(-1);

   // Park if needed, otherwise move backwards
   if (park)
   {
      turnLeft(RIGHTANGLETURN + 30, 80, redAlliance);

      forwardCoast(4000, 127);
      assignDriveMotorsPower(-30, -30);
      delay(100);
      assignDriveMotorsPower(0, 0);

      delay(3000);

      turnRight(RIGHTANGLETURN, 100, redAlliance);
      assignDriveMotorsPower(70, 70);
      delay(300);
      assignDriveMotorsPower(0, 0);

      backward(400, 50);

      forwardCoast(2900, 127);
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

void mainAutonFront(bool redAlliance)
{
   //setFlywheelSpeed(HIGHFLAGPOWER - 10, HIGHFLAGRPM - 60);
   setFlywheelSpeed(MIDDLEFLAGPOWER, MIDDLEFLAGRPM);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");

   forwardCoast(200, 40);
   forwardCoast(1850, 127);
   forwardCoast(200, 10);

   assignDriveMotorsPower(-10, -10 - DIFFBRAKE);
   delay(110);
   backwardCoast(200, 60);
   backwardCoast(1850, 127);

   backwardCoast(600, 80);
   assignDriveMotorsPower(-30, -30);
   delay(500);
   assignDriveMotorsPower(0, 0);

   setIndexerPower(-127);
   delay(120);
   setIndexerPower(FRONTTILEPOWER);

   // Turn and shoot the balls
   forward(130, 70);
   turnRight(373, 60, redAlliance);
   rpmShot = true;
   rapidFire = true;
   delay(300);
   setFlywheelSpeed(HIGHFLAGPOWER - 6, HIGHFLAGRPM - 36);

   forward(1230, 70);
   motor_move(PORT_FLYWHEEL, -127);
   scraperInUseAuton = true;
   delay(900);
   indexerInUseAuton = true;
   setIndexerPower(-127);
   delay(400);
   indexerInUseAuton = false;

   motor_move(PORT_FLYWHEEL, -15);

   assignDriveMotorsPower(-40, -40);
   delay(350);
   assignDriveMotorsPower(0, 0);
   delay(500);
   motor_move(PORT_FLYWHEEL, -6);

   backward(540, 127);
   scraperInUseAuton = false;
   delay(200);

   turnLeft(365, 60, redAlliance);

   assignDriveMotorsPower(-55, -55);
   delay(600);
   assignDriveMotorsPower(0, 0);

   forward(160, 100);
   setIndexerPower(-127);
   delay(100);
   setIndexerPower(0);
   turnRight(RIGHTANGLETURN, 70, redAlliance);

   forward(140, 80);
   rpmShot = false;
   rapidFire = true;
   delay(500);

   scraperInUseAuton = false;
   forwardCoast(200, 127);
   forwardCoast(1200, 127);
   motor_move_relative(PORT_FLYWHEEL, 0, 50);
}

void newBackAuton(bool redAlliance)
{
   setFlywheelSpeed(HIGHFLAGPOWER - 5, HIGHFLAGRPM - 30);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");

   forwardCoast(200, 40);
   forwardCoast(1850, 127);
   forwardCoast(200, 10);

   assignDriveMotorsPower(-10, -10 - DIFFBRAKE);
   delay(110);
   backwardCoast(200, 60);

   turnRight(RIGHTANGLETURN + 60, 70, redAlliance);

   indexerInUseAuton = true;
   setIndexerPower(-1);

   assignDriveMotorsPower(60, 60);
   delay(500);
   assignDriveMotorsPower(0, 0);

   delay(200);

   backward(150, 90);
   indexerInUseAuton = true;
   oneSideRight(-550, 100, redAlliance);

   delay(1000);

   setIndexerPower(-127);
   delay(120);
   setIndexerPower(0);
   indexerInUseAuton = false;

   delay(4000);

   backRapidFire = true;
   delay(1000);

   flywheelOff = true;
   indexerInUseAuton = true;

   setIndexerPower(127);
   oneSideRight(800, 100, redAlliance);
   setIndexerPower(127);

   forwardCoast(2000, 120);
   assignDriveMotorsPower(-10, -10);
   delay(100);
   assignDriveMotorsPower(0, 0);
}

void flagAutonBack(bool park, bool redAlliance, bool troll)
{
   uint32_t start = millis();
   setFlywheelSpeed(HIGHFLAGPOWER, HIGHFLAGRPM);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");

   //Flip cap at back
   moveScraper(SCRAPER_DOWN_POS + 70);
   forwardCoast(2500, 100);
   scraperInUseAuton = false;
   forward(200, 100);
   delay(100);
   backward(2900, 200);
   turnRight(RIGHTANGLETURN / 2, 100, redAlliance);

   //Align with wall
   assignDriveMotorsPower(-60, -60);
   delay(300);
   assignDriveMotorsPower(-30, -30);
   delay(200);

   //Get ball, score cap, align on platform
   forward(2350, 200);
   backward(100, 100);
   turnRight(RIGHTANGLETURN, 100, redAlliance);

   assignDriveMotorsPower(60, 60);
   delay(400);

   backward(200, 100);
   oneSideRight(-600, 100, redAlliance);

   while (millis() - start < 13500)
   {
      delay(20);
   }

   setIndexerPower(-127);
   delay(500);
   oneSideRight(600, 100, redAlliance);

   forwardCoast(2000, 127);
   assignDriveMotorsPower(-30, -30);
   delay(100);
   assignDriveMotorsPower(0, 0);
}

void autonomous()
{
   task_t displayInfoTask = task_create(displayInfoAuton, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Info Task");

   //backward(2000, 127);
   //delay(400);
   //turnLeft(RIGHTANGLETURN, 127, false);
   ///  return;

   newBackAuton(true);
   //fullAutonFront(false, true, true, false);
   return;

   switch (autonNumber)
   {
   case 1:
      mainAutonFront(redAlliance);
      break;
   case 2:
      fullAutonFront(false, redAlliance, true, false);
      break;
   case 3:
      newBackAuton(redAlliance);
      break;
   default:
      break;
   }
}