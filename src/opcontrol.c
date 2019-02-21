#include "main.h"

int intakeDirection = 0;
int indexerDirection = 0;
int currentFlywheelPower = 0;
int currentFlywheelGoalRPM = 0;
int currentAssignedFlywheelPower = 0;
int middleFlagXCoord = 0;
bool knownRPM;

bool doubleFire = false;
bool singleFire = false;
int numVisionObjects = 0;

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

vision_object_s_t getTopFlag(int sigNum)
{
   vision_object_s_t object_arr[NUM_VISION_OBJECTS];
   vision_color_code_t curCode = vision_create_color_code(PORT_VISION, VISIONTARGETSIG, sigNum, 0, 0, 0);
   vision_read_by_code(PORT_VISION, 0, curCode, NUM_VISION_OBJECTS, object_arr);
   //vision_read_by_sig(PORT_VISION, 0, 1, 2, object_arr);

   numVisionObjects = sizeof(object_arr) / sizeof(object_arr[0]);

   if (numVisionObjects == 0 || numVisionObjects > 2)
   {
      vision_object_s_t temp = {.x_middle_coord = 0};
      return temp;
   }
   if (numVisionObjects == 1)
   {
      object_arr[0].x_middle_coord = (int)object_arr[0].left_coord + (int)object_arr[0].width / 2;
      return object_arr[0];
   }
   else
   {
      if (object_arr[0].y_middle_coord > object_arr[1].y_middle_coord)
      {
         printf("Readings: \n");
         printf("%d\n", object_arr[0].left_coord);
         printf("%d\n", object_arr[0].top_coord);
         printf("%d\n", object_arr[0].height);
         printf("%d\n", object_arr[0].width);
         object_arr[0].x_middle_coord = (int)object_arr[0].left_coord + (int)object_arr[0].width / 2;
         return object_arr[0];
      }
      else
      {
         printf("Readings: \n");
         printf("%d\n", object_arr[1].left_coord);
         printf("%d\n", object_arr[1].top_coord);
         printf("%d\n", object_arr[1].height);
         printf("%d\n", object_arr[1].width);
         object_arr[1].x_middle_coord = (int)object_arr[1].left_coord + (int)object_arr[1].width / 2;
         return object_arr[1];
      }
   }
}

void turnToFlag(int sigNum)
{
   vision_object_s_t flag = getTopFlag(sigNum);

   if (numVisionObjects == 0)
   {
      return;
   }

   if (flag.x_middle_coord > VISION_FOV_WIDTH / 2 + ANGLETOLERANCE)
   {
      assignDriveMotorsDControl(40, -40);
      while (flag.x_middle_coord > VISION_FOV_WIDTH / 2 && flag.x_middle_coord > 0 && flag.x_middle_coord < VISION_FOV_WIDTH)
      {
         printf("%d\n", flag.x_middle_coord);
         if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_LEFT))
         {
            delay(200);
            return;
         }
         flag = getTopFlag(sigNum);
         delay(20);
      }
      assignDriveMotorsDControl(-23, 23);
   }
   else if (flag.x_middle_coord < VISION_FOV_WIDTH / 2 - ANGLETOLERANCE)
   {
      assignDriveMotorsDControl(-40, 40);
      while (flag.x_middle_coord < VISION_FOV_WIDTH / 2 && flag.x_middle_coord > 0 && flag.x_middle_coord < VISION_FOV_WIDTH)
      {
         if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_LEFT))
         {
            delay(200);
            return;
         }
         flag = getTopFlag(sigNum);
         delay(20);
      }
      assignDriveMotorsDControl(23, -23);
   }

   delay(100);
   assignDriveMotorsDControl(0, 0);
}

void moveToFlag(int sigNum)
{
   vision_object_s_t flag = getTopFlag(sigNum);
   if (numVisionObjects == 0)
   {
      return;
   }

   if (flag.height < FLAGPIXELHEIGHT - HEIGHTTOLERANCE)
   {
      assignDriveMotorsDControl(40, 40);
      while (flag.height < FLAGPIXELHEIGHT)
      {
         if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_LEFT))
         {
            return;
         }
         printf("%d\n", (int)flag.height);
         flag = getTopFlag(sigNum);
         delay(20);
      }
      assignDriveMotorsDControl(-23, -23);
   }
   else if (flag.height > FLAGPIXELHEIGHT + HEIGHTTOLERANCE)
   {
      assignDriveMotorsDControl(-40, -40);
      while (flag.height > FLAGPIXELHEIGHT)
      {
         if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_LEFT))
         {
            return;
         }
         printf("%d\n", (int)flag.height);
         flag = getTopFlag(sigNum);
         delay(20);
      }
      assignDriveMotorsDControl(23, 23);
   }

   delay(100);
   assignDriveMotorsDControl(0, 0);
}

void newVisionAlign(int sigNum)
{
   turnToFlag(sigNum);
   moveToFlag(sigNum);
   turnToFlag(sigNum);
   getTopFlag(sigNum);

   if (!controller_get_digital(CONTROLLER_MASTER, DIGITAL_LEFT))
   {
      if (numVisionObjects == 2)
      {
         doubleFire = true;
      }
      if (numVisionObjects == 1)
      {
         singleFire = true;
      }
   }
}

void drive(void *param)
{
   while (true)
   {
      if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_LEFT))
      {
         int sigNum = BLUEFLAGSIG;
         if (!redAlliance)
         {
            sigNum = REDFLAGSIG;
         }
         newVisionAlign(sigNum);
      }

      int forward = controller_get_analog(CONTROLLER_MASTER, ANALOG_LEFT_Y);
      int turn = controller_get_analog(CONTROLLER_MASTER, ANALOG_RIGHT_X);

      motor_move(PORT_DRIVELEFTFRONT, max(-127, min(127, forward + turn)));
      motor_move(PORT_DRIVERIGHTFRONT, max(-127, min(127, forward - turn)));
      motor_move(PORT_DRIVELEFTBACK, max(-127, min(127, forward + turn)));
      motor_move(PORT_DRIVERIGHTBACK, max(-127, min(127, forward - turn)));
      motor_move(PORT_DRIVELEFTCENTER, max(-127, min(127, forward + turn)));
      motor_move(PORT_DRIVERIGHTCENTER, max(-127, min(127, forward - turn)));

      delay(20);
   }
}

void assignIndexerFree(int power)
{
   if (indexerDirection == 0)
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
      else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_A))
      {
         currentFlywheelPower = MIDDLEFLAGPOWER;
         currentFlywheelGoalRPM = MIDDLEFLAGRPM;
         currentAssignedFlywheelPower = MIDDLEFLAGPOWER;
         motor_move(PORT_FLYWHEEL, currentFlywheelPower);
         assignIndexerFree(currentFlywheelPower + FRICTIONPOWER);
         knownRPM = true;
         integral = 0;
      }
      else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_Y))
      {
         currentFlywheelPower = BETWEENFLAGPOWER;
         currentFlywheelGoalRPM = BETWEEENFLAGRPM;
         currentAssignedFlywheelPower = BETWEENFLAGPOWER;
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
      else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_RIGHT) || singleFire || doubleFire)
      {
         //rapid fire
         indexerDirection = 1;
         motor_move(PORT_INDEXER, -127); //100
         motor_move(PORT_FLYWHEEL, currentFlywheelPower + EXTRAPOWER);

         while (abs(motor_get_actual_velocity(PORT_FLYWHEEL)) > currentFlywheelGoalRPM - 8)
         {
            if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_A) ||
                controller_get_digital(CONTROLLER_MASTER, DIGITAL_B) ||
                controller_get_digital(CONTROLLER_MASTER, DIGITAL_X) ||
                controller_get_digital(CONTROLLER_MASTER, DIGITAL_Y))
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
            delay(20);
         }

         if (!singleFire)
         {
            motor_move(PORT_FLYWHEEL, -3);
            currentFlywheelGoalRPM = MIDDLEFLAGRPM;
            currentFlywheelPower = MIDDLEFLAGPOWER;
            currentAssignedFlywheelPower = MIDDLEFLAGPOWER;
            delay(94);
            motor_move(PORT_FLYWHEEL, currentFlywheelPower);
            delay(1000);
         }

         motor_move(PORT_INDEXER, currentFlywheelPower + FRICTIONPOWER);
         indexerDirection = 0;
         firstIter = true;

         singleFire = false;
         doubleFire = false;

         armed = 0;
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

         motor_move(PORT_FLYWHEEL, currentAssignedFlywheelPower);
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
         motor_move(PORT_INDEXER, -127);
         motor_move(PORT_FLYWHEEL, currentAssignedFlywheelPower + EXTRAPOWER);
         indexerDirection = 1;
         while (controller_get_digital(CONTROLLER_MASTER, DIGITAL_L1) == 1)
         {
            delay(20);
            //wait
         }
         motor_move(PORT_INDEXER, max(currentAssignedFlywheelPower, 0));
         motor_move(PORT_FLYWHEEL, currentAssignedFlywheelPower);
         indexerDirection = 0;

         if (armed > 0)
         {
            armed -= 1;
         }
      }
      if (adi_digital_read(LIMITSWITCHPORT) == 1 && armed < 2)
      {
         delay(150);
         if (adi_digital_read(LIMITSWITCHPORT) == 1)
         {
            indexerDirection = 1;

            if (armed == 0)
            {
               motor_move(PORT_INDEXER, -127);
               delay(140);
               motor_move(PORT_INDEXER, 0);
            }
            else
            {
               motor_move(PORT_INDEXER, -127);
               delay(200);
               motor_move(PORT_INDEXER, 0);
            }

            indexerDirection = 0;
            assignIndexerFree(currentFlywheelPower + FRICTIONPOWER);

            armed += 1;
         }
      }
      else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_DOWN))
      {
         armed = 0;
      }
      else if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_UP))
      {
         armed = 1;
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
      sprintf(tempString3, "Limit Switch: %d", adi_digital_read(LIMITSWITCHPORT));
      sprintf(tempString4, "Cur Power: %d", currentAssignedFlywheelPower);
      sprintf(tempString5, "Indexer Direction: %d", indexerDirection);
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

   //delay(500);
   task_t driveTask = task_create(drive, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Drive Task");
   task_t flywheelTask = task_create(flywheel, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Flywheel Task");
   task_t indexerTask = task_create(indexer, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Indexer Task");
   task_t displayInfoTask = task_create(displayInfo, "PROS", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Display Info Task");

   //lvglInfo();
}
