

// ================================================================
// ===                        DEFINITIONS                       ===
// ================================================================

#include <stdint.h>
#include "SentralMM.h"
#include "I2Cdev.h"


//Uncomment "WRITE_TO_CLOUD" to print to SD card
// #define TRANSMIT

//Uncomment "DEBUG_CODE" to print to SD card
// #define DEBUG_CODE

// uncomment "OUTPUT_READABLE_EULER" if you want to see Euler angles
// (in degrees) calculated from the quaternions coming from the FIFO.
// Note that Euler angles suffer from gimbal lock (for more info, see
// http://en.wikipedia.org/wiki/Gimbal_lock)
// #define OUTPUT_READABLE_EULER

// uncomment "OUTPUT_READABLE_YAWPITCHROLL" if you want to see the yaw/
// pitch/roll angles (in degrees) calculated from the quaternions coming
// from the FIFO. Note this also requires gravity vector calculations.
// Also note that yaw/pitch/roll angles suffer from gimbal lock (for
// more info, see: http://en.wikipedia.org/wiki/Gimbal_lock)
 #define OUTPUT_READABLE_YAWPITCHROLL

// uncomment "OUTPUT_READABLE_REALACCEL" if you want to see acceleration
// components with gravity removed. This acceleration reference frame is
// not compensated for orientation, so +X is always +X according to the
// sensor, just without the effects of gravity. If you want acceleration
// compensated for orientation, us OUTPUT_READABLE_WORLDACCEL instead.
//#define OUTPUT_READABLE_REALACCEL

// uncomment "OUTPUT_READABLE_WORLDACCEL" if you want to see acceleration
// components with gravity removed and adjusted for the world frame of
// reference (yaw is relative to initial orientation, since no magnetometer
// is present in this case). Could be quite handy in some cases.
// #define OUTPUT_READABLE_WORLDACCEL


#define LED_PIN 3 // LED Indicator pin
#define BUTTON_INTERRUPT 2
#define SD_CS_PIN 10
#define TCAADDR 0x70 // Defines the MUX address
#define MAX_NUM_IMUS 7 // Defines the total number of IMUS connected to the MUX
bool blinkState = false;
bool calibrate_IMUs = false;
float userGyroOffset[MAX_NUM_IMUS][3];
unsigned long timer = 0;
const uint8_t PWMVal = 20;

SentralMM sentral[MAX_NUM_IMUS];
// Sentral control/status vars
bool* SentralReady = new bool[MAX_NUM_IMUS];
uint8_t* devStatus = new uint8_t[MAX_NUM_IMUS];      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t* fifoCount = new uint16_t[MAX_NUM_IMUS]; // Define containers in form of arrays to recieve fifo count from gyro fifo register;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
int eepromAddress; // Calibration offsets from the eeprom are going to be stored here.

// orientation/motion vars
Quaternion q[MAX_NUM_IMUS];           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector

#ifdef OUTPUT_READABLE_EULER
float euler[3]; // [psi, theta, phi]    Euler angle container
#endif
#ifdef OUTPUT_READABLE_YAWPITCHROLL
float ypr[3]; // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
#endif

/******************************FUNCTION DEFINITION****************************/

void collectData(SentralMM gy, int i);
void tcaselect(uint8_t i);


// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup()
{
  long tStart = millis();
  // #############----------- PINMODES ---------->>>>
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_INTERRUPT, INPUT);
  
  // <<<<------- PINMODES END --------------

  analogWrite(LED_PIN, (blinkState * PWMVal) ); 
  
  // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties

// ########## Begins Wifi module protocol if Transmit is defined

// Displays extra info through wifi module to Debug the code
#ifdef DEBUG_CODE
  Serial.begin(115200);
  while (!Serial); // wait for Arduino serial to be ready
#endif 

  // ##############----------- INTERRUPT ATTACH ------------------->
  attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT), calibrate_IMUs, FALLING);

  // <------------ INTERRUPT ATTACH END --------------


  // ###########----------- Get Current Time and perhaps date ------------->

  // <---------- Done getting Time and date --------------

  
  // Ensure 3 second wait for configuration file to be written to the Sentral from Sensor-Embedded EEPROM
  while(millis() - tStart < 3000){} 
  

  for (uint8_t i = 0; i < MAX_NUM_IMUS; i++)
  {
    userGyroOffset[i][0] = 0.0; userGyroOffset[i][1] = 0.0; userGyroOffset[i][2] = 0.0;
    
    tcaselect(i);
    SentralReady[i] = false;

    // initialize device
    uint8_t init_val = sentral[i].initialize();

    // Check to see if there was an error in initialization
    if(init_val != 0){
      #ifdef DEBUG_CODE
      while(1){
        Serial.print("Failed to Initialize sensor ");
        Serial.print((char)(i + 65));
        Serial.print(" . Error Value is: ");
        Serial.println(ini_val);
      }
      #endif
    }else{ sentralReady[i] = true; }

    sentral[i].setSensorRates( 20, 2, 2);
    //sentral[i].setQRateDivisor(1); //Not necessary, default value causes Qrate = GRate 

    // Returns to Normal Operations and enables Scaled Sensor Data and Quaternion in NED convention
    // Not Neccessary since does the above by default
    //sentral[i].setAlgControl(0x00);

    sentral[i].setErrorIntEvent(true);
    sentral[i].setNormalState(true);
  
    // This loop gets the gyro & accel offsets (3 values each)
    int offsetContainer[6];
    eepromAddress = 12 * i + 15;
    for (int xVal = 0; xVal < 6; xVal++)
    {
      EEPROM.get(eepromAddress, offsetContainer[xVal]);
      //Serial.print("Addr-");
      //Serial.print(eepromAddress);
      //Serial.print("\tValue: ");
      //Serial.println(offsetContainer[xVal]);
      eepromAddress += sizeof(int);
    }
  }
  
  timer = millis();
}


// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop()
{
  if(calibrate_IMUs)
  {
    calibrate_IMUs2User();
  }
  else
  {
    // blink LED to indicate activity
      blinkState = !blinkState;
      analogWrite(LED_PIN, (blinkState * PWMVal) ); 
      
      unsigned long currTime = millis();
      
      #ifdef WRITE_TO_SERIALPORT
        //File sdFile;
        Serial.print(millis() - timer);
        Serial.print('/');
      #endif

      #ifdef WRITE_TO_CLOUD

      #endif
    
      for (uint8_t i = 0; i < MAX_NUM_IMUS; i++)
      {
        tcaselect(i);
        collectData(sdFile, i);
    
        if (i < MAX_NUM_IMUS - 1)
        {
          #ifdef WRITE_TO_SERIALPORT
                Serial.print('/');
          #endif
          
          #ifdef WRITE_TO_CLOUD
                sdFile.print('/');
          #endif
        }
      }
    
      #ifdef WRITE_TO_SERIALPORT
        Serial.println();
      #endif
      
      #ifdef WRITE_TO_CLOUD
        sdFile.println();
        sdFile.close();
      #endif
    
      if( ( millis() - currTime) < 50)
      {
        unsigned long t = millis();
        while( (t - currTime) < 50)
        
        {
          t = millis();
        }
      }
    
      if((millis() - timer) > 120000)
      {
        analogWrite(LED_PIN, PWMVal ); 
        while(1){}
      }
  }
  
}


// ================================================================
// ===                    Function Definitions                  ===
// ================================================================

/* Data Acquisition functinn for the gyros
  @Param i - Multiplexer (TCA) Port
*/
void collectData(int i)
{

  // if programming failed, don't try to do anything
  if (!SentralReady[i])return;
  
  fifoCount[i] = sentral[i].getFIFOCount();

  // wait for correct available data length, should be a VERY short wait
  while (fifoCount[i] < packetSize)
    fifoCount[i] = sentral[i].getFIFOCount();

  // read a packet from FIFO
  sentral[i].getFIFOBytes(fifoBuffer, packetSize);

  // track FIFO count here in case there is > 1 packet available
  // (this lets us immediately read more without waiting for an interrupt)
  fifoCount[i] -= packetSize;

  #ifdef WRITE_TO_SERIALPORT
      Serial.print((char)(i + 65));
      Serial.print('|');
  #endif
  
  #ifdef WRITE_TO_CLOUD
      sdFile.print((char)(i + 65));
      sdFile.print('|');
  #endif
  
  #ifdef OUTPUT_READABLE_EULER
      // display Euler angles in degrees
      sentral[i].dmpGetQuaternion(&q, fifoBuffer);
      sentral[i].dmpGetEuler(euler, &q);
  
    #ifdef WRITE_TO_SERIALPORT
        Serial.print((euler[2] * 180 / M_PI) - (userGyroOffset[i][2] * 180 / M_PI)); // Y on gy-521
        Serial.print("|");
        Serial.print((euler[1] * 180 / M_PI) - (userGyroOffset[i][1] * 180 / M_PI)); // X on gy-521
        Serial.print("|");
        Serial.print((euler[0] * 180 / M_PI) - (userGyroOffset[i][0] * 180 / M_PI));
        Serial.print("|");
    #endif
    
    #ifdef WRITE_TO_CLOUD
        sdFile.print((euler[2]- userGyroOffset[i][2]) * 180 / M_PI);
        sdFile.print("|");
        sdFile.print((euler[1] - userGyroOffset[i][1]) * 180 / M_PI);
        sdFile.print("|");
        sdFile.print((euler[0] - userGyroOffset[i][0]) * 180 / M_PI);
        //sdFile.print("|");
    #endif
  #endif
  
  #ifdef OUTPUT_READABLE_YAWPITCHROLL
      // display Euler angles in degrees
      sentral[i].dmpGetQuaternion(&q, fifoBuffer);
      sentral[i].dmpGetGravity(&gravity, &q);
      sentral[i].dmpGetYawPitchRoll(ypr, &q, &gravity);
  
    #ifdef WRITE_TO_SERIALPORT
        Serial.print((ypr[1] - userGyroOffset[i][1]) * -180 / M_PI);
        Serial.print("|");
        Serial.print((ypr[2] - userGyroOffset[i][2]) * 180 / M_PI);
        Serial.print("|");
        Serial.print((ypr[0] - userGyroOffset[i][0]) * -180 / M_PI);
        Serial.print("|");
    #endif
    
    #ifdef WRITE_TO_CLOUD
        sdFile.print((ypr[1] - userGyroOffset[i][1]) * -180 / M_PI);
        sdFile.print("|");
        sdFile.print((ypr[0] - userGyroOffset[i][0]) * -180 / M_PI);
        sdFile.print("|");
        sdFile.print((ypr[2] - userGyroOffset[i][2]) * 180 / M_PI);
        //sdFile.print("|");
    #endif
  #endif
  
  #ifdef OUTPUT_READABLE_REALACCEL
      // display real acceleration, adjusted to remove gravity
      sentral[i].dmpGetQuaternion(&q, fifoBuffer);
      sentral[i].dmpGetAccel(&aa, fifoBuffer);
      sentral[i].dmpGetGravity(&gravity, &q);
      sentral[i].dmpGetLinearAccel(&aaReal, &aa, &gravity);
  
    #ifdef WRITE_TO_SERIALPORT
        Serial.print(aaReal.y * (981.0 / 16384.0), 2); // prints acceleration in y dir, converted from Gs to cm/s^2 and to 2 decimal places
        Serial.print("|");
        Serial.print(aaReal.z * (981.0 / 16384.0), 2); // prints acceleration in z dir, converted from Gs to cm/s^2 and to 2 decimal places
        Serial.print("|");
        Serial.print(aaReal.x * (981.0 / 16384.0), 2); // prints acceleration in x dir, converted from Gs to cm/s^2 and to 2 decimal places
    #endif
    
    #ifdef WRITE_TO_CLOUD
        sdFile.print(aaReal.y * (981.0 / 16384.0), 2); // prints acceleration in y dir, converted from Gs to cm/s^2 and to 2 decimal places
        sdFile.print("|");
        sdFile.print(aaReal.z * (981.0 / 16384.0), 2); // prints acceleration in z dir, converted from Gs to cm/s^2 and to 2 decimal places
        sdFile.print("|");
        sdFile.print(aaReal.x * (981.0 / 16384.0), 2); // prints acceleration in x dir, converted from Gs to cm/s^2 and to 2 decimal places
    #endif
  
  #endif
    
  #ifdef OUTPUT_READABLE_WORLDACCEL
      // display initial world-frame acceleration, adjusted to remove gravity
      // and rotated based on known orientation from quaternion
      sentral[i].dmpGetQuaternion(&q, fifoBuffer);
      sentral[i].dmpGetAccel(&aa, fifoBuffer);
      sentral[i].dmpGetGravity(&gravity, &q);
      sentral[i].dmpGetLinearAccel(&aaReal, &aa, &gravity);
      sentral[i].dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
  
    #ifdef WRITE_TO_SERIALPORT
        Serial.print(aaWorld.y * (981.0 / 16384.0), 2); // prints acceleration in y dir, converted from Gs to cm/s^2 and to 2 decimal places
        Serial.print("|");
        Serial.print(aaWorld.z * (981.0 / 16384.0), 2); // prints acceleration in z dir, converted from Gs to cm/s^2 and to 2 decimal places
        Serial.print("|");
        Serial.print(aaWorld.x * (981.0 / 16384.0), 2); // prints acceleration in x dir, converted from Gs to cm/s^2 and to 2 decimal places
    #endif
    
    #ifdef WRITE_TO_CLOUD
        sdFile.print(aaWorld.y * (981.0 / 16384.0), 2); // prints acceleration in y dir, converted from Gs to cm/s^2 and to 2 decimal places
        sdFile.print("|");
        sdFile.print(aaWorld.z * (981.0 / 16384.0), 2); // prints acceleration in z dir, converted from Gs to cm/s^2 and to 2 decimal places
        sdFile.print("|");
        sdFile.print(aaWorld.x * (981.0 / 16384.0), 2); // prints acceleration in x dir, converted from Gs to cm/s^2 and to 2 decimal places
    #endif
    
  #endif

  // get current FIFO count
  fifoCount[i] = sentral[i].getFIFOCount();
    
  if (fifoCount[i] >= 966 )
  {
    // reset so we can continue cleanly
    sentral[i].resetFIFO();
      #ifdef WRITE_TO_SERIALPORT
            Serial.print("  I: "); Serial.print(i);  Serial.println(": FIFO Overflow  ");
      #endif
      
//      #ifdef WRITE_TO_CLOUD
//        sdFile.print("  I: "); sdFile.print(i);  sdFile.println(": FIFO Overflow  ");     
//      #endif
  }

}

void tcaselect(uint8_t i)
{
  if (i > MAX_NUM_IMUS - 1) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}


void calibrate_IMUs()
{
  calibrate_IMUs = true;
}


void calibrate_IMUs2User()
{
    analogWrite(LED_PIN, 0 ); 
 
  uint8_t sampleQty = 5;
  float data[3];

   for(uint8_t i = 0; i < MAX_NUM_IMUS; i++)
   {
      //if (!SentralReady[i])return;

      userGyroOffset[i][0] = 0.0; userGyroOffset[i][1] = 0.0; userGyroOffset[i][2] = 0.0;
      
      for (uint8_t s = 0; s < sampleQty; s++)
      {
        tcaselect(i);

    // if programming failed, don't try to do anything
      if (!SentralReady[i])return;
      
      fifoCount[i] = sentral[i].getFIFOCount();
    
      // wait for correct available data length, should be a VERY short wait
      while (fifoCount[i] < packetSize)
        fifoCount[i] = sentral[i].getFIFOCount();
    
      // read a packet from FIFO
      sentral[i].getFIFOBytes(fifoBuffer, packetSize);
    
      // track FIFO count here in case there is > 1 packet available
      // (this lets us immediately read more without waiting for an interrupt)
      fifoCount[i] -= packetSize;

        
        #ifdef OUTPUT_READABLE_EULER
            sentral[i].dmpGetQuaternion(&q, fifoBuffer);
            sentral[i].dmpGetEuler(data, &q);
        #endif
        
        #ifdef OUTPUT_READABLE_YAWPITCHROLL             
            sentral[i].dmpGetQuaternion(&q, fifoBuffer);
            sentral[i].dmpGetGravity(&gravity, &q);
            sentral[i].dmpGetYawPitchRoll(data, &q, &gravity);
            
//          Serial.print("Data [");Serial.print(i); Serial.print("][1]:  ");
//          Serial.println(data[1]* 180 / M_PI);
//          Serial.print("Data [");Serial.print(i); Serial.print("][2]:  ");
//          Serial.println(data[2]* 180 / M_PI);
//          Serial.print("Data [");Serial.print(i); Serial.print("][0]:  ");
//          Serial.println(data[0]* 180 / M_PI);
//          Serial.println();
        #endif
        
        for(uint8_t j = 0; j < 3; j++)
        {
          userGyroOffset[i][j] += data[j];
        }
      }
      
      for(uint8_t j = 0; j < 3; j++)
      {
        userGyroOffset[i][j] /= sampleQty;
      }
  
          Serial.print("Offset [");Serial.print(i); Serial.print("][1]:  ");
          Serial.println(userGyroOffset[i][1]* 180 / M_PI);
          Serial.print("Offset [");Serial.print(i); Serial.print("][2]:  ");
          Serial.println(userGyroOffset[i][2]* 180 / M_PI);
          Serial.print("Offset [");Serial.print(i); Serial.print("][0]:  ");
          Serial.println(userGyroOffset[i][0]* 180 / M_PI);
          Serial.println();
   
   }


  #ifdef WRITE_TO_CLOUD  
    SD.remove(sdFilename);
  #endif
  
analogWrite(LED_PIN, PWMVal ); 

  timer = millis();
  calibrate_IMUs = false;
}
