/*
   RED LED blinking - Error from sensors
   Blue LED blinking - Data being transmitted to NODE MCU
   Pink Indicator Blinking - Calibration in progress
   Pink and Red Alternating - Error updating database with currentTime for data logging
   RED On - Not connected to wifi
   RED and BLUE Alternating - Not Calibrated yet
*/

// ================================================================
// ===                        DEFINITIONS                       ===
// ================================================================

#include <stdint.h>
#include "SentralMM.h"
#include "I2Cdev.h"
#include <EEPROM.h>


//Uncomment "WRITE_TO_CLOUD" to print to SD card
#define TRANSMIT

//Uncomment "DEBUG" to print to SD card
#define DEBUG

// ########### Pin Definitions ############
#define BLUE_LED 2 // LED Indicator pin
#define RED_LED 3 // LED Indicator pin
#define VIB1_PIN 4
#define VIB2_PIN 5
#define VIB3_PIN 6
#define VIB4_PIN 7
#define BUTTON_INT 8
#define SENTRAL0_INT 23
#define SENTRAL1_INT 25
#define SENTRAL2_INT 26
#define SENTRAL3_INT 27
#define SENTRAL4_INT 28
#define SENTRAL5_INT 29
#define SENTRAL6_INT 30
// ########### End of Pin Definitions ############

#define TCAADDR 0x70 // Defines the MUX address
#define NUM_IMUS 7 // Defines the total number of IMUS connected to the MUX

bool blinkState[2] = {false, false};
bool calibrate_Data = false;
bool calibrated_Data = false;
uint8_t sampleQty = 5;


unsigned long tStart  = 0;
unsigned long timer = 0;
const uint8_t PWMVal = 180;

SentralMM sentral[NUM_IMUS];
// Sentral control/status vars
bool sentralReady[NUM_IMUS];
uint8_t sentralErr[NUM_IMUS] = {0, 0, 0, 0, 0, 0, 0}; // FIFO storage buffer
int eepromAddress = 0; // Calibration offsets from the eeprom are going to be stored here.
double waitTimer[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
uint8_t magRate = 100; //30;
uint8_t accelRate = 10; //4;
uint8_t gyroRate = 10; //4; //2;
uint8_t quatDivisor = 1;

// orientation/motion vars
Quaternion sentral_q[NUM_IMUS];    // [w, x, y, z] Sentral orien wrt World - quaternion container from sensors starting from sentral A -> sentral G
Quaternion initial_q[NUM_IMUS];    // [w, x, y, z] Initial joint orien wrt World - quaternion container for inital joint orientations from unity model starting from Joint A -> Joint G
Quaternion s2j_q[NUM_IMUS];        // [w, x, y, z] Sensor Value wrt Joint - quaternion container for relationship orientations of sentral quats to joint quats [S2J quats]
//e.g elbow IMU wrt elbow joint) from  A -> G
Quaternion joint_q[NUM_IMUS];      // [w, x, y, z] Joint orien wrt World - quaternion container for Final joint orientations from unity model starting from Joint A -> Joint G

float ypr[3]; // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector


/******************************FUNCTION DEFINITION****************************/

void tcaselect(uint8_t i);
void setupSensor(int i);
void troubleshoot_Err(int i);
void calibrateData();
void checkINT0();
void checkINT1();
void checkINT2();
void checkINT3();
void checkINT4();
void checkINT5();
void checkINT6();



// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup()
{
  // #############----------- PINMODES ---------->>>>
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUTTON_INT, INPUT);
  pinMode(SENTRAL0_INT, INPUT);
  pinMode(SENTRAL1_INT, INPUT);
  pinMode(SENTRAL2_INT, INPUT);
  pinMode(SENTRAL3_INT, INPUT);
  pinMode(SENTRAL4_INT, INPUT);
  pinMode(SENTRAL5_INT, INPUT);
  pinMode(SENTRAL6_INT, INPUT);

  // <<<<------- PINMODES END --------------


  // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties

  // ########## Begins Wifi module protocol if Transmit is defined

  // Displays extra info through wifi module to Debug the code
#ifdef DEBUG
  Serial.begin(9600);
  //while (!Serial); // wait for Arduino serial to be ready
#endif

#ifdef TRANSMIT
  Serial4.begin(9600);
  //while (!Serial); // wait for Arduino serial to be ready
#endif


  // Ensure 3 second wait for configuration file to be written to the Sentral from Sensor-Embedded EEPROM
  pause(3000); //while (millis()< 3000) {} //- tStart


  for (uint8_t i = 0; i < NUM_IMUS; i++)
  {
    tcaselect(i);
    //Serial.println(" Before Setting Up");
    setupSensor(i);
    //Serial.println(" After Setting Up");

    // This loop gets initial quaternion of the model on unity that corresponds to the users T-Pose
    EEPROM.get(eepromAddress, initial_q[i].w); eepromAddress += 4;
    EEPROM.get(eepromAddress, initial_q[i].x); eepromAddress += 4;
    EEPROM.get(eepromAddress, initial_q[i].y); eepromAddress += 4;
    EEPROM.get(eepromAddress, initial_q[i].z); eepromAddress += 4;


#ifdef DEBUG
    //    Serial.print((char)(i + 65)); Serial.print(",");
    //    Serial.print(initial_q[i].w); Serial.print(","); //Serial.print("\t");
    //    Serial.print(initial_q[i].x); Serial.print(",");
    //    Serial.print(initial_q[i].y); Serial.print(",");
    //    Serial.print(initial_q[i].z);
    //    if (i < NUM_IMUS - 1)
    //      Serial.print("/");
    //    else if (i >= NUM_IMUS - 1)
    //      Serial.println();
#endif
  }

  timer = millis(); // Used for timing the program i.e when to stop running.
  pause(500); // Allows the sensor to update first few data in registers

  String tempStr = "";
  String tTempStr = "";
  while (1)
  {
    if (Serial4.available() > 0)
    {
      char chr = Serial4.read();
      //Serial.print("first chr: "); Serial.println(chr);
      
      if (chr == '/')
      {
        tempStr = "";
      }
      else if (chr == '\\')
      { Serial.print("tempStr: "); Serial.println(tempStr);
        if (tempStr == "connected")
        {          
          tTempStr = tempStr;
          tempStr = "";
//          blinkState[0] = true;
//          blinkState[1] = false;
          //break;
        }
        else if (tempStr == "disconnected")
        {
          digitalWrite(RED_LED, HIGH);
        }
       
        
        if (tempStr == "noTimeStamp" && tTempStr == "connected")
        {
          digitalWrite(RED_LED, HIGH);
          while (1)
          {
            digitalWrite(BLUE_LED, HIGH);
            delay(1000);
            digitalWrite(BLUE_LED, LOW);
            delay(1000);
          }
        }
        else if(tempStr == "yesTimeStamp" && tTempStr == "connected")
        {
          Serial.println("yesTimeStamp and Connected");
          tStart = millis();
          blinkState[0] = true;
          blinkState[1] = false;
          digitalWrite(BLUE_LED, blinkState[0]);
          digitalWrite(RED_LED, blinkState[1]);
          break; 
        }
      }
      else
      {
        tempStr += chr;
      }
    }
  
  }

 // ##############----------- INTERRUPT ATTACH ------------------->
  attachInterrupt(digitalPinToInterrupt(BUTTON_INT), calibrateData, LOW);
  attachInterrupt(digitalPinToInterrupt(SENTRAL0_INT), checkINT0, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL1_INT), checkINT1, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL2_INT), checkINT2, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL3_INT), checkINT3, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL4_INT), checkINT4, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL5_INT), checkINT5, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL6_INT), checkINT6, RISING);


  // <------------ INTERRUPT ATTACH END --------------
  #ifdef DEBUG
Serial.println("Out!");
#endif
}

bool calibratedOnce = false;

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop()
{
  if (!calibratedOnce)
  {
    
    if ((millis() - waitTimer[4]) > 150) // blink LED to indicate activity
    {Serial.println("In Calibrated Once");
      blinkState[0] = !blinkState[0];
      blinkState[1] = !blinkState[1];
      digitalWrite(BLUE_LED, blinkState[0]);
      digitalWrite(RED_LED, blinkState[1]);
      waitTimer[4] = millis();
    }
  }

  if (calibrate_Data)
  {
    Serial.println("In calData2User");
    calData2User();
  }
  else if (!calibrate_Data)
  {
    if (calibratedOnce == true)
    {
      //Serial.println("In MainCode");
      if ((millis() - waitTimer[2]) > 500) // blink LED to indicate activity
      {
        blinkState[0] = !blinkState[0];
        digitalWrite(BLUE_LED, blinkState[0]);
        waitTimer[2] = millis();
      }


      // Get Data every 50 millisecond through WIFI
      if ((millis() - waitTimer[1]) > 200)
      {
#ifdef TRANSMIT      
      Serial4.print(millis() - tStart); Serial4.print("|");
#endif

#ifdef DEBUG
      Serial.print(millis() - tStart); Serial.print("|");
#endif
        for (uint8_t i = 0; i < NUM_IMUS; i++)
        {
          tcaselect(i);
          
          if (sentralErr[i] == 1)
          {
            troubleshoot_Err(i);
            sentralErr[i] = 0;
            break;
          }

          // if programming failed, don't try to do anything
          if (!sentralReady[i])return;

          uint8_t eventStatus = sentral[i].getIntStatus();
          //Serial.print("Event Status: "); Serial.println(eventStatus,DEC);

          if ((eventStatus & 0x02) == 0x02) //If there's an error status, restart the sentral
          {
            sentral[i].restartSentral();
            setupSensor(i);
            pause(100);
            //eventStatus = sentral[i].getIntStatus();
            break;
          } else if ((eventStatus & 0x04) == 0x04)
          {
            sentral[i].getQuat(&sentral_q[i]);

#ifdef DEBUG
//            //Serial.print("QW:   ");
//            Serial.print((char)(i + 65)); //Serial.print(",");
//            Serial.print(sentral_q[i].w); Serial.print(","); //Serial.print("\t");
//            //Serial.print("QX:   ");
//            Serial.print(sentral_q[i].x); Serial.print(",");
//            //Serial.print("QY:   ");
//            Serial.print(sentral_q[i].y); Serial.print(",");
//            //Serial.print("QZ:   ");
//            Serial.print(sentral_q[i].z); Serial.print(",");
////            if (i < NUM_IMUS - 1)
////              Serial.print("/");
////            else 
//            if (i >= NUM_IMUS - 1)
//              Serial.println();
#endif

#ifdef DEBUG
            //Serial.print("QW:   ");
            Serial.print((char)(i + 65)); //Serial.print(",");
            Serial.print(joint_q[i].w); Serial.print(","); //Serial.print("\t");
            //Serial.print("QX:   ");
            Serial.print(joint_q[i].x); Serial.print(",");
            //Serial.print("QY:   ");
            Serial.print(joint_q[i].y); Serial.print(",");
            //Serial.print("QZ:   ");
            Serial.print(joint_q[i].z); Serial.print(",");
//            if (i < NUM_IMUS - 1)
//              Serial.print("/");
//            else 
            if (i >= NUM_IMUS - 1)
              Serial.println();
#endif

            joint_q[i] = s2j_q[i].getProduct(sentral_q[i]); // This line gets the true rotations of the joints based on the relationship with initial orien and sentral orien

#ifdef TRANSMIT
   // Data Format: 24593|A0.03,0.94,-0.32,0.15,B0.18,-0.07,-0.46,0.87,C0.71,0.60,-0.22,-0.30,D0.32,-0.18,0.23,0.90,E0.16,-0.76,-0.63,0.02,F0.36,0.06,0.07,0.93,G0.03,-0.83,-0.55,0.02,
            Serial4.print((char)(i + 65));
            Serial4.print(joint_q[i].w); Serial4.print(",");
            Serial4.print(joint_q[i].x); Serial4.print(",");
            Serial4.print(joint_q[i].y); Serial4.print(",");
            Serial4.print(joint_q[i].z); Serial4.print(",");
#endif

          } else
          { //Serial.print("Event Status: "); Serial.println(eventStatus);
            break;
          }
        }
        waitTimer[1] = millis();
      }

      if ((millis() - timer) > 120000)
      {
        digitalWrite(BLUE_LED, HIGH);
        digitalWrite(RED_LED, LOW);
        while (1) {}
      }

    }

  }
}


// ================================================================
// ===                    Function Definitions                  ===
// ================================================================

void setupSensor(int i)
{
  tcaselect(i);

  sentralReady[i] = false;

  // initialize device
  uint8_t init_val = sentral[i].initialize(5000);

  // Check to see if there was an error in initialization
  if (init_val != 0) {
#ifdef DEBUG
    while (1) {
      Serial.print("Failed to Initialize sensor ");
      Serial.print((char)(i + 65));
      Serial.print(" . Error Value is: ");
      Serial.println(init_val);
      if ((millis() - waitTimer[0]) > 200) // blink LED to indicate activity
      {
        blinkState[0] = false;
        blinkState[1] = !blinkState[1];
        digitalWrite(BLUE_LED, blinkState[0]);
        digitalWrite(RED_LED, blinkState[1]);
        waitTimer[0] = millis();
      }
    }
#endif
  } else {
    #ifdef DEBUG
    Serial.print("Sensor Initialized! - ");
    Serial.println((char)(i + 65));
    #endif
    sentralReady[i] = true;
  }

  sentral[i].setSensorRates(magRate, accelRate, gyroRate);
  //sentral[i].setQRateDivisor(quatDivisor); //Not necessary, default value causes Qrate = GRate

  // Returns to Normal Operations and enables Scaled Sensor Data and Quaternion in NED convention
  // Not Neccessary since does the above by default
  //sentral[i].setAlgControl(0x00);

  sentral[i].setErrorIntEvent(true);
  sentral[i].setNormalState(true);
  pause(100);
}

void tcaselect(uint8_t i)
{
  if (i > NUM_IMUS - 1) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}


void calibrateData()
{
  if(digitalRead(BUTTON_INT) == 0)
  {
    calibrate_Data = true;
    Serial.print("Bttn Status: "); Serial.println(digitalRead(BUTTON_INT));
  }
}

void checkINT0() {
  sentralErr[0] = 1;
  Serial.println("Interrupt A triggered");
}
void checkINT1() {
  sentralErr[1] = 1;
  Serial.println("Interrupt B triggered");
}
void checkINT2() {
  sentralErr[2] = 1;
  Serial.println("Interrupt C triggered");

}
void checkINT3() {
  sentralErr[3] = 1;
  Serial.println("Interrupt D triggered");
}
void checkINT4() {
  sentralErr[4] = 1;
  Serial.println("Interrupt E triggered");
}
void checkINT5() {
  sentralErr[5] = 1;
  Serial.println("Interrupt F triggered");
}
void checkINT6() {
  sentralErr[6] = 1;
  Serial.println("Interrupt G triggered");
}


void calData2User()
{
  blinkState[0] = true;
  blinkState[1] = true;
  digitalWrite(BLUE_LED, blinkState[0]);
  digitalWrite(RED_LED, blinkState[1]);
  pause(500);

  Quaternion q_Cum;//[NUM_IMUS];
  Quaternion q_Avg;//[NUM_IMUS];

  for (uint8_t i = 0; i < NUM_IMUS; i++)
  {
    q_Cum.w = 0.0f; // This is needed since quaternion initializes with qw = 1, but it also affects the average calculations
    Quaternion q[sampleQty];
    for (uint8_t s = 0; s < sampleQty; s++)
    {

      tcaselect(i);
      if ((millis() - waitTimer[2]) > 300) // blink LED to indicate activity
      {
        blinkState[0] = !blinkState[0];
        blinkState[1] = !blinkState[1];
        digitalWrite(BLUE_LED, blinkState[0]);
        digitalWrite(RED_LED, blinkState[1]);
        waitTimer[2] = millis();
      }

      // if programming failed, don't try to do anything
      if (!sentralReady[i])
      {
#ifdef DEBUG
        Serial.println("User Calibration: Sentral Not Ready!");
#endif
        return;
      }

      uint8_t eventStatus = sentral[i].getIntStatus();

      //      Serial.print("eventStatus: ");
      //      Serial.println(eventStatus);

      if ((eventStatus & 0x01) == 0x01)
      {
#ifdef DEBUG
        Serial.println("User Calibration: Sentral still starting!");
#endif
        sentral[i].restartSentral();
        pause(1000);
        setupSensor(i);
        eventStatus = sentral[i].getIntStatus();
        pause(60);
        s--; continue;
      }
      else if ((eventStatus & 0x02) == 0x02) //If there's an error status, restart the sentral
      {
#ifdef DEBUG
        Serial.println("User Calibration: Sentral Has Error!");
#endif
        troubleshoot_Err(i);
        sentral[i].restartSentral();
        pause(1000);
        setupSensor(i);
        //eventStatus = sentral[i].getIntStatus();
        s--;
        //pause(100);
        pause(60);
        continue;
      } else if
      ((eventStatus & 0x04) == 0x04)
      {
        sentral[i].getQuat(&q[s]);
#ifdef DEBUG
        Serial.print("QW: ");
        Serial.print(q[s].w); Serial.print("\t");
        Serial.print("QX: ");
        Serial.print(q[s].x); Serial.print("\t");
        Serial.print("QY: ");
        Serial.print(q[s].y); Serial.print("\t");
        Serial.print("QZ: ");
        Serial.println(q[s].z);
#endif
      }
      else
      {
#ifdef DEBUG
        Serial.println("User Calibration: Some other Reason!");
#endif
        s--;
        //pause(120);
        pause(50);
        continue;
      }

      if (sampleQty >= 2)
      {
        // Finds the average of the quaternions for each sensor
        q_Avg = averageQuat(&q_Cum, q[s], q[0], s );
      }
      else
      {
        q_Avg = q[s];
      }

      waitTimer[3] = millis();
      while (s < sampleQty - 1 && ((millis() - waitTimer[3]) < 60)) {}
    }

#ifdef DEBUG
    Serial.print("QW_AVG:   ");
    Serial.print(q_Avg.w); Serial.print("\t");
    Serial.print("QX_AVG:   ");
    Serial.print(q_Avg.x); Serial.print("\t");
    Serial.print("QY_AVG:   ");
    Serial.print(q_Avg.y); Serial.print("\t");
    Serial.print("QZ_AVG:   ");
    Serial.println(q_Avg.z);
#endif

    s2j_q[i] = calcS2JQuat(initial_q[i], q_Avg); //[i]);

#ifdef DEBUG
    Serial.print("QW_CORR: ");
    Serial.print(s2j_q[i].w); Serial.print("\t");
    Serial.print("QX_CORR: ");
    Serial.print(s2j_q[i].x); Serial.print("\t");
    Serial.print("QY_CORR: ");
    Serial.print(s2j_q[i].y); Serial.print("\t");
    Serial.print("QZ_CORR: ");
    Serial.println(s2j_q[i].z);
#endif

    q_Avg.resetQuat();
  }

  calibrate_Data = false;
  calibratedOnce = true;

  blinkState[0] = true;
  blinkState[1] = false;
  digitalWrite(BLUE_LED, blinkState[0]);
  digitalWrite(RED_LED, blinkState[1]);
  pause(500);
  timer = millis();

}

void troubleshoot_Err(int i)
{
  if (sentral[i].getResetStatus() == 1)
  {
#ifdef DEBUG
    Serial.println("\n****ERROR: Sentral Configuration File Needs Uploading! ****\n");
#endif
  }
  if (sentral[i].getEEUploadErr() == 1)
  {
#ifdef DEBUG
    Serial.println("\n****ERROR: Issue with uploading from EEPROM! ****\n");
#endif
  }

  if (sentral[i].getGRate() == 0)
  {
#ifdef DEBUG
    Serial.println("\n****ERROR: Gyro Data Rate is Zero! ****\n");
#endif
  }

  if (sentral[i].getMRate() == 0)
  {
#ifdef DEBUG
    Serial.println("\n****ERROR: Magnetometer Data Rate is Zero! ****\n");
#endif
  }

  if (sentral[i].getARate() == 0)
  {
#ifdef DEBUG
    Serial.println("\n****ERROR: Accelerometer Data Rate is Zero! ****\n");
#endif
  }

#ifdef DEBUG
  Serial.println("\n****Attempting to Restart Sensor! ****\n");
#endif
  sentral[i].restartSentral();
  delay(2000);
  setupSensor(i);
}

// Pauses in Milliseconds
void pause(int milli)
{
  long timPause = millis();
  while ((millis() - timPause) < milli) { }
}

/* Returns the quaternion that represents the relationship between the initial joint orientation
    and the IMU's (sentral's) orientation.
    e.g the quat of the imu of elbow wrt elbow = initial quat wrt world (quat multiplied by) the conjugate of sentral quat wrt World
*/
Quaternion calcS2JQuat(Quaternion initialQuat, Quaternion sentralQuat)
{
  return initialQuat.getProduct(sentralQuat.getConjugate());
}

float rad2deg(float rad)
{
  rad * (180 / M_PI);
}

// ######### Quat Avergaing Stuff - J.Ojo ################
float dotProduct(Quaternion q1, Quaternion q2)
{
  return q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
}

bool AreQuaternionsClose(Quaternion q1, Quaternion q2) {

  float dot = dotProduct(q1, q2);

  if (dot < 0.0f) {

    return false;
  }

  else {

    return true;
  }
}


// http://wiki.unity3d.com/index.php/Averaging_Quaternions_and_Vectors
//Get an average (mean) from more than two quaternions (with two, slerp would be used).
//Note: this only works if all the quaternions are relatively close together.
//Usage:
//-Cumulative is an external Vector4 which holds all the added x y z and w components.
//-newRotation is the next rotation to be added to the average pool
//-firstRotation is the first quaternion of the array to be averaged
//-addAmount holds the total amount of quaternions which are currently added
//This function returns the current average quaternion
Quaternion averageQuat(Quaternion *cumulative, Quaternion newRotation, Quaternion firstRotation, int addAmount) {

  float w = 0.0f;
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;

  //Before we add the new rotation to the average (mean), we have to check whether the quaternion has to be inverted. Because
  //q and -q are the same rotation, but cannot be averaged, we have to make sure they are all the same.
  if (!AreQuaternionsClose(newRotation, firstRotation)) {

    newRotation.flipQuatSign();
  }

  //Average the values
  float addDet = 1.0f / (float)addAmount;
  cumulative -> w += newRotation.w;
  w = cumulative -> w * addDet;
  cumulative -> x += newRotation.x;
  x = cumulative -> x * addDet;
  cumulative -> y += newRotation.y;
  y = cumulative -> y * addDet;
  cumulative -> z += newRotation.z;
  z = cumulative -> z * addDet;

  //note: if speed is an issue, you can skip the normalization step
  return Quaternion(w, x, y, z).getNormalized();
}

