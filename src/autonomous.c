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

uint32_t programStartTime = 0;

#define RIGHTANGLETURN 950 //780 //780 // 770 //850

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

   while (abs(errorChange) > TDRIVEMAXVEL || (!endSequence || (millis() - startEnd < endTime)))
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
      assignDriveMotorsDist(0, ticks, power, true, true);
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
      assignDriveMotorsDist(ticks, 0, power, true, true);
   }
}

void coast(int ticks, int power, int heading)
{
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
   coast(ticks, power, heading);
}

void backwardCoast(int ticks, int power, int heading)
{
   coast(ticks, -power, heading);
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
      if (millis() - now > 200000 || flywheelOff) //14900
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
            if (!redAlliance)
            {
               delay(150);
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

void fullAutonFront(bool redAlliance)
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

   assignDriveMotorsPower(-50, -50);
   delay(300);
   assignDriveMotorsPower(-30, -30);
   setIndexerPower(-127);
   delay(100);
   indexerInUseAuton = false;
   delay(200);
   assignDriveMotorsPower(0, 0);

   // Move from wall and shoot
   forwardCoast(240, 50, 0);
   turnRight(RIGHTANGLETURN, 70);
   delay(200);
   forwardCoast(140, 70, RIGHTANGLETURN);

   rapidFire = true;
   delay(300);

   forwardCoast(100, 70, RIGHTANGLETURN);
   scraperInUseAuton = false;
   forwardCoast(320, 80, RIGHTANGLETURN);
   forwardCoast(200, 90, RIGHTANGLETURN);

   int tinyTurn = 80;
   //turnRight(RIGHTANGLETURN + tinyTurn, 80);

   forwardCoast(1290, 90, RIGHTANGLETURN + tinyTurn);
   forward(100, 40, RIGHTANGLETURN + tinyTurn);

   setFlywheelSpeed(HIGHFLAGPOWER, HIGHFLAGRPM);
   setIndexerPower(-127);

   // Thwack the low flag
   scraperInUseAuton = true;
   motor_move(PORT_FLYWHEEL, -127);
   delay(300);
   scraperInUseAuton = false;

   backwardCoast(100, 60, RIGHTANGLETURN);
   backwardCoast(1600, 127, RIGHTANGLETURN);
   backwardCoast(350, 40, RIGHTANGLETURN);
   assignDriveMotorsPower(3, 3);
   delay(100);
   assignDriveMotorsPower(0, 0);

   int curTurnedValue = 430;
   turnLeft(curTurnedValue, 60);
   delay(200);

   //Scrape and flip cap
   scraperInUseAuton = true;
   motor_move(PORT_FLYWHEEL, -112);
   indexerInUseAuton = true;
   motor_move(PORT_INDEXER, 0);

   forwardCoast(750, 75, curTurnedValue);
   assignDriveMotorsPower(-2, -2);
   delay(150);
   assignDriveMotorsPower(0, 0);
   motor_move(PORT_FLYWHEEL, -127);
   delay(420);
   motor_move(PORT_FLYWHEEL, -20);

   backwardCoast(50, 40, curTurnedValue);
   indexerInUseAuton = false;
   backwardCoast(100, 40, curTurnedValue);

   motor_move_relative(PORT_FLYWHEEL, 120, 70);
   delay(300);
   motor_move(PORT_FLYWHEEL, -30);
   delay(100);
   motor_move(PORT_FLYWHEEL, -127);
   backward(500, 127, curTurnedValue);

   forwardCoast(300, 110, curTurnedValue);
   scraperInUseAuton = false;
   forwardCoast(290, 70, curTurnedValue + 15);
   assignDriveMotorsPower(-1, -1);
   delay(150);
   assignDriveMotorsPower(0, 0);

   setIndexerPower(-127);
   delay(120);
   indexerInUseAuton = false;

   while (millis() - now < 14600)
   {
      delay(10);
   }

   rapidFire = true;
}

void mainAutonFront(bool redAlliance)
{
   uint32_t now = millis();
   //setFlywheelSpeed(HIGHFLAGPOWER - 10, HIGHFLAGRPM - 60);
   //setFlywheelSpeed(MIDDLEFLAGPOWER, MIDDLEFLAGRPM);
   setFlywheelSpeed(HIGHFLAGPOWER - 14, HIGHFLAGRPM - 75);
}

void newBackAuton(bool redAlliance)
{
   int32_t now = millis();
   setFlywheelSpeed(HIGHFLAGPOWER - 5, HIGHFLAGRPM - 27);
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
      fullAutonFront(redAlliance);
      break;
   case 3:
      newBackAuton(redAlliance);
      break;
   default:
      redAlliance = false;
      fullAutonFront(false);
      break;
   }
}