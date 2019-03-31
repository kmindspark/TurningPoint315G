#include "main.h"

int intakeDirection = 0;
bool indexerInUse = false;
bool scraperInUse = false;
int currentFlywheelPower = 0;
int currentFlywheelGoalRPM = 0;
int currentAssignedFlywheelPower = 0;
int middleFlagXCoord = 0;
bool knownRPM;

bool doubleFire = false;
bool singleFire = false;
int numVisionObjects = 0;

bool readyToMove = false;

int armed = 0;

#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

void assignDriveMotorsDControl(int leftSide, int rightSide)
{
   motor_move(PORT_DRIVELEFTFRONT, leftSide);
   motor_move(PORT_DRIVELEFTBACK, leftSide);
   motor_move(PORT_DRIVELEFTCENTER, leftSide);
   motor_move(PORT_DRIVERIGHTFRONT, rightSide);
   motor_move(PORT_DRIVERIGHTBACK, rightSide);
   motor_move(PORT_DRIVERIGHTCENTER, rightSide);
}

bool anyButtonPressed()
{
   return controller_get_digital(CONTROLLER_MASTER, DIGITAL_A) ||
          controller_get_digital(CONTROLLER_MASTER, DIGITAL_B) ||
          controller_get_digital(CONTROLLER_MASTER, DIGITAL_X) ||
          controller_get_digital(CONTROLLER_MASTER, DIGITAL_Y);
}

void lockDriveMotors()
{
   motor_move_relative(PORT_DRIVELEFTBACK, 0, 200);
   motor_move_relative(PORT_DRIVELEFTCENTER, 0, 200);
   motor_move_relative(PORT_DRIVELEFTFRONT, 0, 200);
   motor_move_relative(PORT_DRIVERIGHTBACK, 0, 200);
   motor_move_relative(PORT_DRIVERIGHTCENTER, 0, 200);
   motor_move_relative(PORT_DRIVERIGHTFRONT, 0, 200);
}

void drive(void *param)
{
   int prevForward = 0;
   int prevTurn = 0;
   while (true)
   {
      int forward = controller_get_analog(CONTROLLER_MASTER, ANALOG_LEFT_Y);
      int turn = controller_get_analog(CONTROLLER_MASTER, ANALOG_RIGHT_X);

      if (prevForward != forward || prevTurn != turn || forward != 0 || turn != 0)
      {
         motor_move(PORT_DRIVELEFTFRONT, max(-127, min(127, forward + turn)));
         motor_move(PORT_DRIVERIGHTFRONT, max(-127, min(127, forward - turn)));
         motor_move(PORT_DRIVELEFTBACK, max(-127, min(127, forward + turn)));
         motor_move(PORT_DRIVERIGHTBACK, max(-127, min(127, forward - turn)));
         motor_move(PORT_DRIVELEFTCENTER, max(-127, min(127, forward + turn)));
         motor_move(PORT_DRIVERIGHTCENTER, max(-127, min(127, forward - turn)));
      }

      prevForward = forward;
      prevTurn = turn;

      delay(20);
   }
}

void assignIndexerFree(int power)
{
   if (indexerInUse == false)
   {
      motor_move(PORT_INDEXER, power);
   }
}

void flywheel(void *param)
{
   bool firstIter = true;
   double integral = 0;
   while (true)
   {
      if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_X) || firstIter)
      {
         firstIter = false;
         currentFlywheelPower = HIGHFLAGPOWER;
         currentFlywheelGoalRPM = HIGHFLAGRPM;
         currentAssignedFlywheelPower = HIGHFLAGPOWER;
         motor_move(PORT_FLYWHEEL, currentFlywheelPower);
         assignIndexerFree(currentFlywheelPower + FRICTIONPOWER);
         knownRPM = true;
         integral = 0;
      }
      else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_B))
      {
         currentFlywheelPower = 0;
         currentFlywheelGoalRPM = 0;
         currentAssignedFlywheelPower = 0;
         motor_move(PORT_FLYWHEEL, currentFlywheelPower);
         assignIndexerFree(currentFlywheelPower + FRICTIONPOWER);
         knownRPM = false;
         integral = 0;
      }
      else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_R1) || singleFire || doubleFire)
      {
         lockDriveMotors();
         //rapid fire
         indexerInUse = true;
         motor_move(PORT_INDEXER, -127); //100
         motor_move(PORT_FLYWHEEL, currentFlywheelPower + EXTRAPOWER);

         int prevVal = adi_digital_read(LIMITSWITCHPORT);

         while (prevVal == adi_digital_read(LIMITSWITCHPORT) || adi_digital_read(LIMITSWITCHPORT) == 1)
         {
            prevVal = adi_digital_read(LIMITSWITCHPORT);
            if (anyButtonPressed())
            {
               break;
            }
            int difference = currentFlywheelGoalRPM - abs(motor_get_actual_velocity(PORT_FLYWHEEL));

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

            currentAssignedFlywheelPower = currentFlywheelPower + EXTRAPOWER + (int)proportional + (int)integral;

            motor_move(PORT_FLYWHEEL, currentAssignedFlywheelPower);
            delay(5);
         }

         delay(50);
         motor_move(PORT_INDEXER, -127);
         scraperInUse = true;
         motor_move(PORT_FLYWHEEL, -127);
         delay(145);
         motor_move_relative(PORT_FLYWHEEL, 100, 50);
         delay(300);
         scraperInUse = false;
         indexerInUse = false;
      }
      if (knownRPM)
      {
         // PID LOOP
         int difference = currentFlywheelGoalRPM - abs(motor_get_actual_velocity(PORT_FLYWHEEL));

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

         currentAssignedFlywheelPower = currentFlywheelPower + (int)proportional + (int)integral;

         if (!scraperInUse)
         {
            motor_move(PORT_FLYWHEEL, currentAssignedFlywheelPower);
         }
         assignIndexerFree(currentAssignedFlywheelPower + FRICTIONPOWER);
      }
      delay(20);
   }
}

void indexer(void *param)
{
   while (true)
   {
      if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_L1) == 1)
      {
         indexerInUse = true;
         lockDriveMotors();
         motor_move(PORT_FLYWHEEL, currentAssignedFlywheelPower + EXTRAPOWER);
         //delay(180);
         motor_move(PORT_INDEXER, -127);
         //delay(80);

         while (controller_get_digital(CONTROLLER_MASTER, DIGITAL_L1) == 1)
         {
            delay(20);
            //wait
         }
         motor_move(PORT_INDEXER, 0);
         indexerInUse = false;
      }
      else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_L2) == 1)
      {
         lockDriveMotors();

         scraperInUse = true;
         motor_move(PORT_FLYWHEEL, -127);
         if (adi_digital_read(LIMITSWITCHPORT) == 1)
         {
            delay(100);
         }

         indexerInUse = true;
         motor_move(PORT_INDEXER, -127);

         while (controller_get_digital(CONTROLLER_MASTER, DIGITAL_L2))
         {
            delay(10);
         }

         indexerInUse = false;
         scraperInUse = false;
      }
      else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_UP))
      {
         lockDriveMotors();
      }
      delay(20);
   }
}

void scraper(void *param)
{
   while (true)
   {
      if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_R2))
      {
         if (!scraperInUse)
         {
            motor_tare_position(PORT_FLYWHEEL);
            scraperInUse = true;
         }

         motor_move(PORT_FLYWHEEL, -127);
         while (controller_get_digital(CONTROLLER_MASTER, DIGITAL_R2))
         {
            delay(10);
         }

         if (motor_get_position(PORT_FLYWHEEL) < SCRAPER_DOWN_POS)
         {
            motor_move(PORT_FLYWHEEL, -4);
            //motor_move_absolute(PORT_FLYWHEEL, SCRAPER_DOWN_POS, 50);
         }
         else
         {
            scraperInUse = false;
         }
      }
      if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_Y))
      {
         scraperInUse = false;
      }

      delay(20);
   }
}

void displayInfo(void *param)
{
   lcd_initialize();
   while (true)
   {
      char tempString1[100];
      char tempString2[100];
      char tempString3[100];
      char tempString4[100];
      char tempString5[100];
      char tempString6[100];

      sprintf(tempString1, "Flywheel Temperature: %d", (int)motor_get_temperature(PORT_FLYWHEEL));
      sprintf(tempString2, "Current Flywheel RPM: %d", abs(motor_get_actual_velocity(PORT_FLYWHEEL)));
      sprintf(tempString3, "Indexer Temperature: %d", (int)motor_get_temperature(PORT_INDEXER));
      sprintf(tempString4, "Cur Power: %d", currentAssignedFlywheelPower);
      sprintf(tempString5, "Indexer Direction: %d", indexerInUse);
      sprintf(tempString6, "Battery Voltage: %d", battery_get_voltage());

      lcd_set_text(1, tempString1);
      lcd_set_text(2, tempString2);
      lcd_set_text(3, tempString3);
      lcd_set_text(4, tempString4);
      lcd_set_text(5, tempString5);
      lcd_set_text(6, tempString6);

      //controller_print(CONTROLLER_MASTER, 0, 0, "RPM: %.2f", motor_get_actual_velocity(PORT_FLYWHEEL));

      delay(10);
   }
}

void opcontrol()
{
   task_t driveTask = task_create(drive, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Drive Task");
   task_t flywheelTask = task_create(flywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   task_t indexerTask = task_create(indexer, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Indexer Task");
   task_t scraperTask = task_create(scraper, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Indexer Task");
   task_t displayInfoTask = task_create(displayInfo, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Info Task");

   //lvglInfo();
}
