#include "main.h"

void turnLeftEnc(int ticks, int power, bool reversed);
void turnRightEnc(int ticks, int power, bool reversed);
void oneSideLeftEnc(int ticks, int power, bool reversed);
void oneSideRightEnc(int ticks, int power, bool reversed);

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
bool ballShot = false;

uint32_t programStartTime = 0;

#define RIGHTANGLETURN 950 //780 //780 // 770 //850
#define OLDRIGHTANGLETURN 715

adi_gyro_t gyro;

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

void resetGyro()
{
   adi_gyro_reset(gyro);
}

int getHeading()
{
   return adi_gyro_get(gyro) - GYRODRIFTRATE * (millis() - programStartTime) * 0.001;
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
          currentPrecision * NUMDRIVEMOTORS) // ||
                                             //(turn && (averageVelocity() >= VELOCITYLIMIT)))
   {
      delay(20);
   }

   delay(250);
   assignDriveMotorsPower(0, 0);
}

void straightPID(int leftSide, int rightSide, int power, int heading)
{
   leftSide *= 200.0 / 257.0;
   rightSide *= 200.0 / 257.0;
   power *= 200.0 / 257.0;

   if (redAlliance)
   {
      heading = -heading;
   }

   clearDriveMotors();

   double curP = DRIVEP;
   double curI = DRIVEI;
   double curD = DRIVED;
   double CORR = CORRD;

   int leftErrorChange = 0;
   int rightErrorChange = 0;
   int leftError = 3 * leftSide;
   int rightError = 3 * rightSide;
   int prevLeftError = 3 * leftSide;
   int prevRightError = 3 * rightSide;
   int leftIntegral = 0;
   int rightIntegral = 0;

   bool endSequence = false;
   int startEnd = 0;
   int endTime = 500;

   while (abs(leftErrorChange) + abs(rightErrorChange) > DRIVEMAXVEL || (!endSequence || (millis() - startEnd < endTime)))
   {
      if (abs(leftError) + abs(rightError) <= DRIVEPOSTOL && !endSequence)
      {
         endSequence = true;
         startEnd = millis();
      }

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

      int rightCorrection = heading - getHeading();
      assignDriveMotorsPower(min(power, max(-power, leftError * DRIVEP + leftIntegral * DRIVEI + leftErrorChange * DRIVED + rightCorrection * CORR)), min(power, max(-power, rightError * DRIVEP + rightIntegral * DRIVEI + rightErrorChange * DRIVED - rightCorrection * CORR)));

      delay(5);

      leftError = 3 * leftSide - motor_get_position(PORT_DRIVELEFTFRONT) - motor_get_position(PORT_DRIVELEFTCENTER) - motor_get_position(PORT_DRIVELEFTBACK);
      rightError = 3 * rightSide - motor_get_position(PORT_DRIVERIGHTFRONT) - motor_get_position(PORT_DRIVERIGHTCENTER) - motor_get_position(PORT_DRIVERIGHTBACK);
   }
   assignDriveMotorsPower(0, 0);
}

void turnGyro(int degrees, int power)
{
   uint32_t cur = millis();
   if (redAlliance)
   {
      degrees = -degrees;
   }

   double curP = TDRIVEP;
   double curI = TDRIVEI;
   double curD = TDRIVED;

   int errorChange = 0;
   int error = degrees - getHeading();
   int prevError = degrees - getHeading();
   int integral = 0;

   bool endSequence = false;
   int startEnd = 0;
   int endTime = 500;

   while ((abs(errorChange) > TDRIVEMAXVEL || (!endSequence || (millis() - startEnd < endTime))) && millis() - cur < 1000)
   {
      if (abs(error) <= TDRIVEPOSTOL && !endSequence)
      {
         endSequence = true;
         startEnd = millis();
      }

      integral += error;
      if (integral > TDRIVEINTEGRALLIMIT)
      {
         integral = TDRIVEINTEGRALLIMIT;
      }
      else if (integral < -TDRIVEINTEGRALLIMIT)
      {
         integral = -TDRIVEINTEGRALLIMIT;
      }

      errorChange = error - prevError;
      prevError = error;

      delay(5);

      error = degrees - getHeading();
      int powerToAssign = min(power, max(-power, error * TDRIVEP + integral * TDRIVEI + errorChange * TDRIVED));
      assignDriveMotorsPower(powerToAssign, -powerToAssign);
   }
   assignDriveMotorsPower(0, 0);
}

void turnLeftEnc(int ticks, int power, bool reversed)
{
   if (reversed)
   {
      turnRightEnc(ticks, power, false);
   }
   else
   {
      assignDriveMotorsDist(-ticks, ticks, power, true, true);
   }
}

void turnRightEnc(int ticks, int power, bool reversed)
{
   if (reversed)
   {
      turnLeftEnc(ticks, power, false);
   }
   else
   {
      assignDriveMotorsDist(ticks, -ticks, power, true, true);
   }
}

void oneSideLeftEnc(int ticks, int power, bool reversed)
{

   if (reversed)
   {
      oneSideRightEnc(ticks, power, false);
   }
   else
   {
      assignDriveMotorsDist(0, ticks * 200.0 / 257.0, power, true, true);
   }
}

void oneSideRightEnc(int ticks, int power, bool reversed)
{
   if (reversed)
   {
      oneSideLeftEnc(ticks, power, false);
   }
   else
   {
      assignDriveMotorsDist(ticks * 200.0 / 257.0, 0, power, true, true);
   }
}

void coast(int ticks, int power, int heading, bool corr)
{
   ticks *= 200.0 / 257.0;
   power *= 200.0 / 257.0;

   if (redAlliance)
   {
      heading = -heading;
   }

   int error = 0;

   clearDriveMotors();
   assignDriveMotorsPower(power, power);
   while (abs(motor_get_position(PORT_DRIVELEFTFRONT)) + abs(motor_get_position(PORT_DRIVELEFTBACK)) +
              abs(motor_get_position(PORT_DRIVERIGHTFRONT)) + abs(motor_get_position(PORT_DRIVERIGHTBACK)) +
              abs(motor_get_position(PORT_DRIVELEFTCENTER)) + abs(motor_get_position(PORT_DRIVERIGHTCENTER)) <
          ticks * NUMDRIVEMOTORS)
   {
      error = heading - getHeading();
      if (!corr)
      {
         error = 0;
      }
      assignDriveMotorsPower(power + error * CORRD, power - error * CORRD);
      delay(5);
   }
   assignDriveMotorsPower(0, 0);
}

void forward(int ticks, int power, int heading)
{
   straightPID(ticks, ticks, power, heading);
}

void backward(int ticks, int power, int heading)
{
   straightPID(-ticks, -ticks, power, heading);
}

void forwardCoast(int ticks, int power, int heading)
{
   coast(ticks, power, heading, true);
}

void backwardCoast(int ticks, int power, int heading)
{
   coast(ticks, -power, heading, true);
}

void forwardCoastNoCorr(int ticks, int power)
{
   coast(ticks, power, 0, false);
}

void backwardCoastNoCorr(int ticks, int power)
{
   coast(ticks, -power, 0, false);
}

void turnLeft(int degrees, int power)
{
   turnGyro(degrees, power);
}

void turnRight(int degrees, int power)
{
   turnGyro(degrees, power);
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
   motor_move(PORT_FLYWHEEL, 50);
   motor_move(PORT_INDEXER, 50);
   delay(300);
   motor_move(PORT_FLYWHEEL, 80);
   motor_move(PORT_INDEXER, 80);
   delay(600);
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

   int32_t now = millis();

   while (true)
   {
      if (millis() - now > 14900 || flywheelOff) //14900
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

         ballShot = true;

         delay(00);

         if (rpmShot)
         {
            delay(20);
         }

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
            if (true)
            {
               delay(800);
            }
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
      sprintf(tempString4, "Gyro Reading: %d", (int)getHeading());
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

void fullAutonFront(bool redAlliance, bool middleCol)
{
   programStartTime = millis();
   uint32_t now = programStartTime;

   // Start the flywheel
   setFlywheelSpeed(HIGHFLAGPOWER + 2, HIGHFLAGRPM + 10);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");

   // Pick up ball from cap
   forwardCoast(200, 60, 0);
   forwardCoast(1550, 127, 0);
   forwardCoast(300, 80, 0);
   forwardCoast(200, 10, 0);

   delay(250);

   backwardCoast(200, 30, 0);
   backwardCoast(1800, 127, 0);
   backwardCoast(600, 80, 0);

   assignDriveMotorsPower(-30, -30);
   delay(300);
   assignDriveMotorsPower(-20, -20);
   setIndexerPower(-127);
   delay(100);
   indexerInUseAuton = false;
   delay(200);
   assignDriveMotorsPower(0, 0);

   // Move from wall and shoot
   forwardCoast(300, 50, 0);
   turnRight(RIGHTANGLETURN, 70);
   delay(600);
   forwardCoast(280, 70, RIGHTANGLETURN);

   rapidFire = true;
   delay(300);
   if (middleCol)
   {
      setFlywheelSpeed(HIGHFLAGPOWER + 30, HIGHFLAGRPM + 150);
   }
   else
   {
      setFlywheelSpeed(BACKTILEPOWER - 10, BACKTILERPM - 50);
   }

   forwardCoast(60, 70, RIGHTANGLETURN);
   scraperInUseAuton = false;
   forwardCoast(220, 80, RIGHTANGLETURN);
   forwardCoast(200, 90, RIGHTANGLETURN);
   setIndexerPower(-127);

   int tinyTurn = 80;
   //turnRight(RIGHTANGLETURN + tinyTurn, 80);

   forwardCoast(1550, 90, RIGHTANGLETURN + tinyTurn); //1400
   assignDriveMotorsPower(-3, -3);
   delay(150);
   assignDriveMotorsPower(0, 0);
   //forward(100, 40, RIGHTANGLETURN + tinyTurn);

   delay(300);

   // Thwack the low flag
   /*scraperInUseAuton = true;
   motor_move(PORT_FLYWHEEL, -127);
   delay(400);
   scraperInUseAuton = false;*/

   backwardCoast(100, 60, RIGHTANGLETURN);
   backwardCoast(1830, 127, RIGHTANGLETURN);
   backwardCoast(350, 40, RIGHTANGLETURN);
   assignDriveMotorsPower(3, 3);
   delay(100);
   assignDriveMotorsPower(0, 0);

   int curTurnedValue = 440;
   turnLeft(curTurnedValue, 60);
   delay(200);

   //Scrape and flip cap
   scraperInUseAuton = true;
   motor_move(PORT_FLYWHEEL, -100);
   indexerInUseAuton = true;
   motor_move(PORT_INDEXER, 0);

   forwardCoast(860, 90, curTurnedValue);
   assignDriveMotorsPower(-2, -2);
   delay(150);
   assignDriveMotorsPower(0, 0);
   motor_move(PORT_FLYWHEEL, -127);
   delay(380);

   backwardCoast(50, 30, curTurnedValue);
   indexerInUseAuton = false;
   motor_move(PORT_FLYWHEEL, -20);
   backwardCoast(120, 60, curTurnedValue);

   motor_move_relative(PORT_FLYWHEEL, 120, 70);
   delay(300);
   motor_move(PORT_FLYWHEEL, -30);
   delay(100);
   motor_move(PORT_FLYWHEEL, -80);
   backward(500, 127, curTurnedValue);

   motor_move(PORT_FLYWHEEL, -15);
   forwardCoast(300, 110, curTurnedValue);
   scraperInUseAuton = false;
   if (!middleCol)
   {
      forwardCoastNoCorr(370, 70);
   }
   forwardCoastNoCorr(670, 70);
   assignDriveMotorsPower(-3, -3);
   delay(150);
   assignDriveMotorsPower(0, 0);

   if (middleCol)
   {

      setFlywheelSpeed(HIGHFLAGPOWER + 2, HIGHFLAGRPM + 10);

      setIndexerPower(-127);
      delay(100);
      indexerInUseAuton = false;

      while (millis() - now < 14600)
      {
         delay(10);
      }

      rapidFire = true;
   }
   else
   {
      int turnAmt = 234;
      turnRight(turnAmt, 80);

      setIndexerPower(-110); //-127
      int prevVal = adi_digital_read(LIMITSWITCHPORT);
      while (prevVal == adi_digital_read(LIMITSWITCHPORT) || adi_digital_read(LIMITSWITCHPORT) == 1)
      {
         prevVal = adi_digital_read(LIMITSWITCHPORT);
         delay(5);
      }

      backwardCoast(1000, 10, turnAmt);
   }
}

void mainAutonFront(bool redAlliance)
{
   uint32_t now = millis();

   setFlywheelSpeed(HIGHFLAGPOWER - 14, HIGHFLAGRPM - 75);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");

   // Pick up ball from cap
   forwardCoast(200, 60, 0);
   forwardCoast(1550, 127, 0);
   forwardCoast(300, 80, 0);
   forwardCoast(200, 10, 0);

   delay(250);

   backwardCoast(200, 30, 0);
   backwardCoast(1800, 127, 0);
   backwardCoast(600, 80, 0);

   assignDriveMotorsPower(-30, -30);
   delay(300);
   assignDriveMotorsPower(-20, -20);
   setIndexerPower(-127);
   delay(100);
   indexerInUseAuton = false;
   delay(200);
   assignDriveMotorsPower(0, 0);

   forwardCoast(50, 70, 0);
   delay(200);

   int curTurnedValue = 455; //475
   turnRight(curTurnedValue, 80);

   indexerInUseAuton = true;
   motor_move(PORT_INDEXER, 0);

   forwardCoast(1030, 60, curTurnedValue);
   delay(300);
   backward(198, 70, curTurnedValue);

   indexerInUseAuton = false;
   rapidFire = true;

   delay(450);
   motor_move(PORT_FLYWHEEL, -127);
   delay(800);

   //Scrape balls and flip cap
   backwardCoast(50, 30, curTurnedValue);
   indexerInUseAuton = false;
   motor_move(PORT_FLYWHEEL, -20);
   backwardCoast(150, 60, curTurnedValue);

   motor_move_relative(PORT_FLYWHEEL, 120, 70);
   delay(300);
   motor_move(PORT_FLYWHEEL, -30);
   delay(100);
   motor_move(PORT_FLYWHEEL, -127);
   backward(500, 127, curTurnedValue);

   motor_move(PORT_FLYWHEEL, -15);
   forwardCoast(300, 110, curTurnedValue);
   scraperInUseAuton = false;
   forwardCoast(290, 70, curTurnedValue);
   assignDriveMotorsPower(-1, -1);
   delay(150);
   assignDriveMotorsPower(0, 0);

   if (redAlliance)
   {
      assignDriveMotorsPower(-85, -120);
   }
   else
   {
      assignDriveMotorsPower(-120, -85);
   }

   delay(750);
   assignDriveMotorsPower(-40, -40);
   delay(650);

   forwardCoastNoCorr(240, 70);
   setIndexerPower(-127);
   delay(80);
   setIndexerPower(0);
   turnRightEnc(RIGHTANGLETURN + 10, 100, redAlliance);

   while (millis() - now < 13600)
   {
      delay(10);
   }

   rpmShot = false;
   rapidFire = true;
   delay(500);

   scraperInUseAuton = false;
   if (redAlliance)
   {
      assignDriveMotorsPower(107, 127);
   }
   else
   {
      assignDriveMotorsPower(127, 107);
   }

   delay(1000);
   assignDriveMotorsPower(0, 0);
}

void newBackAuton(bool redAlliance)
{
   int32_t now = millis();
   setFlywheelSpeed(BACKTILEPOWER, BACKTILERPM);
   task_t flywheelTask = task_create(autonFlywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");

   // Pick up ball from cap
   forwardCoast(200, 60, 0);
   forwardCoast(1550, 127, 0);
   forwardCoast(300, 80, 0);
   forwardCoast(200, 10, 0);

   delay(250);

   if (redAlliance)
   {
      backwardCoast(175, 60, 0);
   }
   else
   {
      backwardCoast(200, 60, 0);
   }

   turnRight(RIGHTANGLETURN, 100);

   indexerInUseAuton = true;
   setIndexerPower(-1);

   assignDriveMotorsPower(60, 60);
   delay(500);
   assignDriveMotorsPower(0, 0);

   delay(200);

   assignDriveMotorsPower(-60, -60);
   delay(170);
   assignDriveMotorsPower(3, 3);
   delay(100);
   indexerInUseAuton = true;

   if (redAlliance)
   {
      oneSideRightEnc(-560, 100, redAlliance);
   }
   else
   {
      oneSideRightEnc(-560, 100, redAlliance);
   }

   indexerInUseAuton = false;
   delay(1000);

   setIndexerPower(-127);
   delay(120);
   setIndexerPower(0);

   indexerInUseAuton = false;

   while (millis() - now < 11000)
   {
      delay(10);
   }

   backRapidFire = true;

   delay(1500);

   flywheelOff = true;
   indexerInUseAuton = true;

   setIndexerPower(127);

   oneSideRightEnc(520, 100, redAlliance);
   setIndexerPower(127);

   forwardCoastNoCorr(2000, 120);
   assignDriveMotorsPower(-10, -10);
   delay(100);
   assignDriveMotorsPower(0, 0);
}

void autonomous()
{
   resetGyro();
   task_t displayInfoTask = task_create(displayInfoAuton, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Info Task");

   switch (autonNumber)
   {
   case 1:
      mainAutonFront(redAlliance);
      break;
   case 2:
      fullAutonFront(redAlliance, true);
      break;
   case 3:
      fullAutonFront(redAlliance, false);
      break;
   case 4:
      newBackAuton(redAlliance);
      break;
   default:
      redAlliance = false;
      newBackAuton(false);
      break;
   }
}