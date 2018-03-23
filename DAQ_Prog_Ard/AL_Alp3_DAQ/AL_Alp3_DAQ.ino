

// ================================================================
// ===                        DEFINITIONS                       ===
// ================================================================

#include <stdint.h>
#include "SentralMM.h"
#include "I2Cdev.h"
#include <EEPROM.h>


//Uncomment "WRITE_TO_CLOUD" to print to SD card
// #define TRANSMIT

//Uncomment "DEBUG_CODE" to print to SD card
 #define DEBUG_CODE

// ########### Pin Definitions ############
#define LED1_PIN LED_BUILTIN // LED Indicator pin
#define LED2_PIN 3 // LED Indicator pin
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
#define NUM_IMUS 1 // Defines the total number of IMUS connected to the MUX

bool blinkState[2] = {false, false};
bool calibrate_Data = false;
bool calibrated_Data = false;

unsigned long timer = 0;
const uint8_t PWMVal = 180;

SentralMM sentral[NUM_IMUS];
// Sentral control/status vars
bool* sentralReady = new bool[NUM_IMUS];
uint8_t* devStatus = new uint8_t[NUM_IMUS];      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t* fifoCount = new uint16_t[NUM_IMUS]; // Define containers in form of arrays to recieve fifo count from gyro fifo register;     // count of all bytes currently in FIFO
uint8_t sentralErr[NUM_IMUS] = {0}; //, 0, 0, 0, 0, 0, 0}; // FIFO storage buffer
int eepromAddress = 0; // Calibration offsets from the eeprom are going to be stored here.
double waitTimer[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
uint8_t magRate = 20;
uint8_t accelRate = 2;
uint8_t gyroRate = 2;
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
  long tStart = millis();
  // #############----------- PINMODES ---------->>>>
  pinMode(LED1_PIN, OUTPUT);
  pinMode(BUTTON_INT, INPUT);

  // <<<<------- PINMODES END --------------

  analogWrite(LED1_PIN, (blinkState[0] * PWMVal) );

  // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();
  Wire.setClock(100000); // 400kHz I2C clock. Comment this line if having compilation difficulties

  // ########## Begins Wifi module protocol if Transmit is defined

  // Displays extra info through wifi module to Debug the code
#ifdef DEBUG_CODE
  Serial.begin(115200);
  while (!Serial); // wait for Arduino serial to be ready
#endif

  // ##############----------- INTERRUPT ATTACH ------------------->
  attachInterrupt(digitalPinToInterrupt(BUTTON_INT), calibrateData, FALLING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL0_INT), checkINT0, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL1_INT), checkINT1, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL2_INT), checkINT2, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL3_INT), checkINT3, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL4_INT), checkINT4, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL5_INT), checkINT5, RISING);
  attachInterrupt(digitalPinToInterrupt(SENTRAL6_INT), checkINT6, RISING);


  // <------------ INTERRUPT ATTACH END --------------


  // ###########----------- Get Current Time and perhaps date ------------->

  // <---------- Done getting Time and date --------------


  // Ensure 3 second wait for configuration file to be written to the Sentral from Sensor-Embedded EEPROM
  while (millis() - tStart < 3000) {}


  for (uint8_t i = 0; i < NUM_IMUS; i++)
  {
    //tcaselect(i);
    //Serial.println(" Before Setting Up");
    setupSensor(i);
    //Serial.println(" After Setting Up");

    // This loop gets initial quaternion of the model on unity that corresponds to the users T-Pose
    //    EEPROM.get(eepromAddress, initial_q[i].w); eepromAddress += 4;
    //    EEPROM.get(eepromAddress, initial_q[i].x); eepromAddress += 4;
    //    EEPROM.get(eepromAddress, initial_q[i].y); eepromAddress += 4;
    //    EEPROM.get(eepromAddress, initial_q[i].z); eepromAddress += 4;
  }

  timer = millis();
}


// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop()
{

  if (false) // calibrate_Data)
  {
    calibrateData();
  }
  else
  {
    if (true) //calibrated_Data == true)
    {
      if ((millis() - waitTimer[2]) > 500) // blink LED to indicate activity
      {
        blinkState[0] = !blinkState[0];
        digitalWrite(LED_BUILTIN, blinkState[0]);
        waitTimer[2] = millis();
      }
      

      // Send Data every 50 millisecond through WIFI
      if ((millis() - waitTimer[1]) > 50)
      {
        for (uint8_t i = 0; i < NUM_IMUS; i++)
        {
          //tcaselect(i);

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
            delay(100);
            //eventStatus = sentral[i].getIntStatus();
            break;
          } else if ((eventStatus & 0x04) == 0x04)
          {//Serial.println("yeye");
            sentral[i].getQuat(&sentral_q[i]);
            Serial.print("QX:   ");Serial.print(sentral_q[i].x);
            Serial.print("\t");
            Serial.print("QY:   ");Serial.print(sentral_q[i].y);
            Serial.print("\t");
            Serial.print("QZ:   ");Serial.print(sentral_q[i].z);
            Serial.print("\t");
            Serial.print("QW:   ");Serial.println(sentral_q[i].w);
            //joint_q[i] = s2j_q[i].getProduct(sentral_q[i]); // This line gets the true rotations of the joints based on the relationship with initial orien and sentral orien
          } else
          { //Serial.print("Event Status: "); Serial.println(eventStatus);
            break;
          }
        }
        waitTimer[1] = millis();
      }

      if ((millis() - timer) > 120000)
      {
        analogWrite(LED1_PIN, PWMVal );
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
  //tcaselect(i);

  sentralReady[i] = false;

  // initialize device
  uint8_t init_val = sentral[i].initialize(5000);

  // Check to see if there was an error in initialization
  if (init_val != 0) {
#ifdef DEBUG_CODE
    while (1) {
      Serial.print("Failed to Initialize sensor ");
      Serial.print((char)(i + 65));
      Serial.print(" . Error Value is: ");
      Serial.println(init_val);
    }
#endif
  } else {
    sentralReady[i] = true;
  }

  sentral[i].setSensorRates(magRate, accelRate, gyroRate);
  //sentral[i].setQRateDivisor(quatDivisor); //Not necessary, default value causes Qrate = GRate

  // Returns to Normal Operations and enables Scaled Sensor Data and Quaternion in NED convention
  // Not Neccessary since does the above by default
  //sentral[i].setAlgControl(0x00);

  sentral[i].setErrorIntEvent(true);
  sentral[i].setNormalState(true);
  delay(100);
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
  calibrate_Data = true;
}

void checkINT0() {
  sentralErr[0] = 1;
}
void checkINT1() {
  sentralErr[1] = 1;
}
void checkINT2() {
  sentralErr[2] = 1;
}
void checkINT3() {
  sentralErr[3] = 1;
}
void checkINT4() {
  sentralErr[4] = 1;
}
void checkINT5() {
  sentralErr[5] = 1;
}
void checkINT6() {
  sentralErr[6] = 1;
}


void calData2User()
{
  uint8_t sampleQty = 5;

  Quaternion q_Cum[NUM_IMUS];
  Quaternion q_Avg[NUM_IMUS];

  for (uint8_t i = 0; i < NUM_IMUS; i++)
  {
    Quaternion q[sampleQty];
    for (uint8_t s = 0; s < sampleQty; s++)
    {
      if ((millis() - waitTimer[0]) > 50)
      {
        //tcaselect(i);

        // if programming failed, don't try to do anything
        if (!sentralReady[i])return;
        uint8_t eventStatus = sentral[i].getIntStatus();

        if ((eventStatus & 0x01) == 1)
        {
          sentral[i].restartSentral();
          delay(100);
          setupSensor(i);
          eventStatus = sentral[i].getIntStatus();
          s--; break;
        } else if ((eventStatus & 0x02) == 1)
        {
          sentral[i].getQuat(&q[s]);
        } else
        {
          s--;
          break;
        }

        if (sampleQty >= 2)
        {
          // Finds the average of the quaternions for each sensor
          q_Avg[i] = averageQuat(&q_Cum[i], q[s], q[0], s + 1);
        }
        else
        {
          q_Avg[i] = q[s];
        }
        waitTimer[0] = millis();
      }
    }

    s2j_q[i] = calcS2JQuat(initial_q[i], q_Avg[i]);

  }
  timer = millis();
  calibrate_Data = false;
}

void troubleshoot_Err(int i)
{
  if (sentral[i].getResetStatus() == 1)
  {
#ifdef DEBUG_CODE
    Serial.println("\n****ERROR: Sentral Configuration File Needs Uploading! ****\n");
#endif
  }
  if (sentral[i].getEEUploadErr() == 1)
  {
#ifdef DEBUG_CODE
    Serial.println("\n****ERROR: Issue with uploading from EEPROM! ****\n");
#endif
  }

  if (sentral[i].getGRate() == 0)
  {
#ifdef DEBUG_CODE
    Serial.println("\n****ERROR: Gyro Data Rate is Zero! ****\n");
#endif
  }

  if (sentral[i].getMRate() == 0)
  {
#ifdef DEBUG_CODE
    Serial.println("\n****ERROR: Magnetometer Data Rate is Zero! ****\n");
#endif
  }

  if (sentral[i].getARate() == 0)
  {
#ifdef DEBUG_CODE
    Serial.println("\n****ERROR: Accelerometer Data Rate is Zero! ****\n");
#endif
  }

#ifdef DEBUG_CODE
  Serial.println("\n****Attempting to Restart Sensor! ****\n");
#endif
  sentral[i].restartSentral();
  setupSensor(i);
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

// ######### Stuff I Added - J.Ojo ################
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
  if (AreQuaternionsClose(newRotation, firstRotation)) {

    newRotation = newRotation.flipQuatSign();
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
  return Quaternion(x, y, z, w).getNormalized();
}

