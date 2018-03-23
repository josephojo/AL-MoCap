

#include "SentralMM.h"

/** Default constructor, uses default I2C address.
 * @see SentralMM_DEFAULT_ADDRESS
 */
SentralMM::SentralMM() {
    devAddr = SentralMM_DEFAULT_ADDRESS;
}


/** Initializes Sentral M&M into initialized state.
 * This function checks to make sure that the configuration file has uploaded into the sentral.
 * @param timeout Optional read timeout in milliseconds (0 to disable, leave off to use default class value)
 * @return 0 = Success, else, error value stored in register( bit 5 -> 0)
 */
uint8_t SentralMM::initialize(uint16_t timeout) {
  
    // Read Sentral status (upload of configuration file from eeprom)
    int stat = 0;
    stat = getSentralStatus();
    #ifdef DEBUG_CODE
        Serial.print("Status1: ");Serial.println(stat);
        uint8_t eeDetect = getEEDetected();
        Serial.print("eeDetect: ");Serial.println(eeDetect);
        uint8_t eeUpDone = getEEUploadDone();
        Serial.print("eeUpDone: ");Serial.println(eeUpDone);
        uint8_t eeERR = getEEUploadErr();
        Serial.print("eeERR: ");Serial.println(eeERR);
     #endif
    
    // Check the Status of the configuration files upload and wait if not completed
        // Check if EEPROM was detected, if not, restart sentral
        if((stat & 0x01) == 0)//eeDetect == 0)
        {
          #ifdef DEBUG_CODE 
            Serial.println("EEPRom not detected!"); 
          #endif
          bool b = restartSentral();
          stat = getSentralStatus();
          #ifdef DEBUG_CODE
            Serial.print("Status2: ");Serial.println(stat); 
          #endif
          if(stat & 0x01 == 0)
            return stat; 
        }

        // Check EEUploadDone bit, and wait until it equals 1 ot time runs out
        uint16_t timer = millis();
        
        while((stat & 0x02) == 0) //eeUpDone == 0)
        {
          #ifdef DEBUG_CODE
            Serial.println("Not done uploading!!!");
            eeUpDone = getEEUploadDone();
            Serial.print("Status3: ");Serial.println(eeUpDone); 
          #endif
          stat = getSentralStatus();
          if((millis() - timer) > timeout)
            return stat;
        }

        // Check EEUpload Error bit to make sure that there was no errors during upload
        // If not restart Sentral and try again
        if((stat & 0x04) == 0x04) //eeERR == 1)
        {
          bool b = restartSentral();
          stat = getSentralStatus();
          #ifdef DEBUG_CODE
              Serial.println("Error while uploading!");
              Serial.print("StatusErr: ");Serial.println(stat); 
              eeERR = getEEUploadErr();
          #endif
          if(stat & 0x04 == 0x04) //eeERR == 1)
            return stat; 
        }

    // If no error exists, returns 0
    return 0;
}

/** Gets the values in the Sentral Status register
 * 
 */
uint8_t SentralMM::getSentralStatus(){
  uint8_t buff[1];
  uint8_t x = I2Cdev::readBits(devAddr, SentralMM_SSTATUS, SentralMM_SS_EE_UPLOAD_ERR_BIT, SentralMM_SS_LENGTH, buff);
  return buff[0];
}

/* Gets the EEProm Upload Done bit from the Sentral Status Register.
 * @return a value 0 / 1. Where 1 means there was the configuration files have uploaded and 0 means otherwise.
 */
uint8_t SentralMM::getEEDetected(){
  uint8_t buff[1];
   I2Cdev::readBit(devAddr, SentralMM_SSTATUS, SentralMM_SS_EE_DET_BIT, buff);
   return buff[0];
}

/* Gets the EEProm Upload Done bit from the Sentral Status Register.
 * @return a value 0 / 1. Where 1 means there was the configuration files have uploaded and 0 means otherwise.
 */
uint8_t SentralMM::getEEUploadDone(){
  uint8_t buff[1];
   I2Cdev::readBit(devAddr, SentralMM_SSTATUS, SentralMM_SS_EE_UPLOADED_BIT, buff);
   return buff[0];
}

/* Gets the EEProm Upload Error bit from the Sentral Status Register.
 * @return a value 0 / 1. Where 1 means there was an error during the configuration files upload and 0 means otherwise.
 */
uint8_t SentralMM::getEEUploadErr(){
  uint8_t buff[1];
  I2Cdev::readBit(devAddr, SentralMM_SSTATUS, SentralMM_SS_EE_UPLOAD_ERR_BIT, buff);
   return buff[0];
}

/* Restarts the sentral
 * Emulates a hard power down/power up
 */
bool SentralMM::restartSentral(){
 return I2Cdev::writeBit(devAddr, SentralMM_RESETREQ, SentralMM_RESETREQ_BIT, true);
}

/* Sets the quaternion data rate divisor which sets the rate of the quat data
 *  Quat Data is set by dividing the Gyro rate by the divisor( gRate/QRateDivisor = QRate)
 *  @Param divisor - Divisor for setting QRate. Divisor = 0 or 1 means QRate = gRate/1.
 */
void SentralMM::setQRateDivisor(uint8_t divisor){
  I2Cdev::writeByte(devAddr, SentralMM_QRATE_DIVISOR, divisor);
}

/* Writes all adata rates to the rate registers
 *  The write starts with the MagRate register since it's the first in the sequence
 *  @Param mRate -> Input value is same as requested value i.e rate = 20 -> ODR = 20 Hz
 *  @Param aRate -> Input value is 1/10th the requested value i.e rate = 30 -> ODR = 300 Hz
 *  @Param gRate -> Input value is 1/10th the requested value i.e rate = 10 -> ODR = 100 Hz 
 */
void SentralMM::setSensorRates(uint8_t mRate, uint8_t aRate,uint8_t gRate ){
  buffer[0] = mRate;
  buffer[1] = aRate;
  buffer[2] = gRate;
  I2Cdev::writeBytes(devAddr, SentralMM_MAGRATE, 3, buffer);
}

/* Sets the Magnetometer output data rate (ODR)
 *  @Param rate -> Input value is same as requested value i.e rate = 20 -> ODR = 20 Hz
 *  The next highest ODR will be used if rate requested are not supported
 */
void SentralMM::setMRate(uint8_t rate){
  I2Cdev::writeByte(devAddr, SentralMM_MAGRATE, rate);
}

/* Sets the Accelerometer output data rate (ODR)
 *  @Param rate -> Input value is 1/10th the requested value i.e rate = 30 -> ODR = 300 Hz
 *  The next highest ODR will be used if rate requested are not supported
 */
void SentralMM::setARate(uint8_t rate){
  I2Cdev::writeByte(devAddr, SentralMM_ACCELRATE, rate);
}

/* Sets the Gyroscope output data rate (ODR)
 *  @Param rate -> Input value is 1/10th the requested value i.e rate = 10 -> ODR = 100 Hz
 *  The next highest ODR will be used if rate requested are not supported
 */
void SentralMM::setGRate(uint8_t rate){
  I2Cdev::writeByte(devAddr, SentralMM_GYRORATE, rate);
}
// -------------
/* Gets the quaternion data rate divisor which sets the rate of the quat data
 *  Quat Data is set by dividing the Gyro rate by the divisor( gRate/QRateDivisor = QRate)
 *  @return divisor - Divisor for setting QRate. Divisor = 0 or 1 means QRate = gRate/1.
 */
uint8_t SentralMM::getQRateDivisor(){
  I2Cdev::readByte(devAddr, SentralMM_QRATE_DIVISOR, buffer);
  return buffer[0];
}

/* Gets the Magnetometer output data rate (ODR)
 *  @return rate -> Input value is same as requested value i.e rate = 20 -> ODR = 20 Hz
 *  The next highest ODR will be used if rate requested are not supported
 */
uint8_t SentralMM::getMRate(){
  I2Cdev::readByte(devAddr, SentralMM_MAGRATE, buffer);
  return buffer[0];
}

/* Gets the Accelerometer output data rate (ODR)
 *  @return rate -> Input value is 1/10th the requested value i.e rate = 30 -> ODR = 300 Hz
 *  The next highest ODR will be used if rate requested are not supported
 */
uint8_t SentralMM::getARate(){
  I2Cdev::readByte(devAddr, SentralMM_ACCELRATE,  buffer);
  return buffer[0];
}

/* Gets the Gyroscope output data rate (ODR)
 *  @return rate -> Input value is 1/10th the requested value i.e rate = 10 -> ODR = 100 Hz
 *  The next highest ODR will be used if rate requested are not supported
 */
uint8_t SentralMM::getGRate(){
  I2Cdev::readByte(devAddr, SentralMM_GYRORATE,  buffer);
  return buffer[0];
}


/* Sets all accesible values in the algorithm control register
 * Default Val is 0. Quaternion output, Scaled Sensor Data, and Normal Operation
 * @Param alg -> 4 = Heading, Pitch, Roll (HPR), Scaled Sensor Data, Normal Operation 
 *               2 = Quaternion output, Raw Data, Normal Operation
 *               1 = Standby Mode
 */
void SentralMM::setAlgControl(uint8_t alg){
  I2Cdev::writeByte(devAddr, SentralMM_ALG_CTRL, alg);
}

/* Sets the type of sensor data the algorithm outputs
 * @Param val -> 1 = Raw data provided in MX, MY, MZ, AX, AY, AZ, GX, GY, & GZ. 0 = Scaled sensor data(uT, g, deg/s).
 */
void SentralMM::setRawDataType(uint8_t val){
  I2Cdev::writeBit(devAddr, SentralMM_ALG_CTRL, SentralMM_ALG_CTRL_RAWDATA_BIT, val);
}

/* Sets the type of sensor data the algorithm outputs
 * @Param val -> 1 = Heading, pitch, and roll output in QX, QY, & QZ. QW = 0.0. 0 = Quaternion outputs
 */
void SentralMM::setAngleType(uint8_t val){
  I2Cdev::writeBit(devAddr, SentralMM_ALG_CTRL, SentralMM_ALG_CTRL_HPR_QUAT_BIT, val);
}

void SentralMM::setIntEvents(uint8_t val){
  I2Cdev::writeBits(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_GYRO_BIT, SentralMM_EN_EVENT_LENGTH, val);
}

uint8_t SentralMM::getIntEvents(){
  I2Cdev::readBits(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_GYRO_BIT, SentralMM_EN_EVENT_LENGTH, buffer);
  return buffer[0];
}

void SentralMM::setErrorIntEvent(bool enabled){
  I2Cdev::writeBit(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_ERR_BIT, enabled);
}

uint8_t SentralMM::getErrorIntEvent(){
  I2Cdev::readBit(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_ERR_BIT, buffer);
  return buffer[0];
}

void SentralMM::setQuatResultIntEvent(bool enabled){
  I2Cdev::writeBit(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_QUAT_BIT, enabled);
}

uint8_t SentralMM::getQuatResultIntEvent(){
  I2Cdev::readBit(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_QUAT_BIT, buffer);
  return buffer[0];
}

void SentralMM::setMagResultIntEvent(bool enabled){
  I2Cdev::writeBit(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_MAG_BIT, enabled);
}

uint8_t SentralMM::getMagResultIntEvent(){
  I2Cdev::readBit(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_MAG_BIT, buffer);
  return buffer[0];
}

void SentralMM::setAccelResultIntEvent(bool enabled){
  I2Cdev::writeBit(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_ACCL_BIT, enabled);
}

uint8_t SentralMM::getAccelResultIntEvent(){
  I2Cdev::readBit(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_ACCL_BIT, buffer);
  return buffer[0];
}

void SentralMM::setGyroResultIntEvent(bool enabled){
  I2Cdev::writeBit(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_GYRO_BIT, enabled);
}

uint8_t SentralMM::getGyroResultIntEvent(){
  I2Cdev::readBit(devAddr, SentralMM_EN_EVENT_INTERRUPT, SentralMM_EN_EVENT_INTERRUPT_GYRO_BIT, buffer);
  return buffer[0];
}

// --------------------------------------------------------------------------------------------
// Pass thru is when the host is speaking directly with individual sensors
//void SentralMM::setPassThruState(bool enabled);
//bool SentralMM::getPassThruState();

// Stanby State configurations
void SentralMM::setStandbyState(bool enabled){
  I2Cdev::writeBit(devAddr, SentralMM_ALG_CTRL, SentralMM_ALG_CTRL_STANDBY_BIT, enabled);
}

bool SentralMM::getStandbyState(){
  I2Cdev::readBit(devAddr, SentralMM_ALG_CTRL, SentralMM_ALG_CTRL_STANDBY_BIT, buffer);
  return buffer[0];
}

/* Get Standby Status
 * 
 */
bool SentralMM::getStandbyStatus(){
  I2Cdev::readBit(devAddr, SentralMM_ALG_STATUS, 0, buffer);
  return buffer[0];
}


// Normal Operation State Configuration
void SentralMM::setNormalState(bool enabled){
  I2Cdev::writeBit(devAddr, SentralMM_HOST_CONTROL, 0, enabled);
}


bool SentralMM::getNormalState(){
  I2Cdev::readBit(devAddr, SentralMM_HOST_CONTROL, 0, buffer);
  return buffer[0];
}
// --------------------------------------------------------------------------------------------

/* Reading the INT Status [EventStatus] clears it. Always read the error bit first before the rest.
 * Reads all documented bits in the register except bit [0], the CPUReset bit from MSB to LSB
 *    In the event of an error, reset the Sentral (@see SentralMM::restartSentral) or you could troublehoot
 *    To troubshoot (@see Sentral::getMRate, Sentral::getARate, Sentral::getGRate for a value of 0x00 meaning a loss of value)
 *        or (@see SentralMM::SentralMM::getEEUploadErr or SentralMM::getResetStatus)
 * @return value a combination of all documented bits except CPUReset as a number (MSB first, LSB last (endianness)) -> [GyroResult, AccelResult, MagResult, QuatResult, Error]
 */
uint8_t SentralMM::getIntStatus(){
  //Serial.print("buffer[0]: "); Serial.println(buffer[0]);
  int y = I2Cdev::readBits(devAddr, SentralMM_INTERRUPT_STATUS, SentralMM_INTERRUPT_STATUS_GYRO_BIT, SentralMM_INT_STATUS_LENGTH, buffer);
//  Serial.print("Num of bits: "); Serial.println(y);
//  Serial.print("buf[0]: "); Serial.println(buf[0]);
  return buffer[0];
}

/* Gets the CPU REset Bit to check to see if the Sentral is in a state where the configuration files haven't been uploaded
 * @return a value 0 / 1. 1 meaning the configuration files aren't uploaded
 */
uint8_t SentralMM::getResetStatus(){
  I2Cdev::readBit(devAddr, SentralMM_INTERRUPT_STATUS, SentralMM_INTERRUPT_STATUS_CPU_RST, buffer);
  return buffer[0];
}

// ---------------- Data Collection From Sensor ---------------------------

/*Gets the quaternion values from the sensor
 * @param qx - The address where the X quaternion component is stored
 * @param qy - The address where the Y quaternion component is stored
 * @param qz - The address where the Z quaternion component is stored
 * @param qw - The address where the W quaternion component is stored
 */
void SentralMM::getQuat(float* qx, float* qy, float* qz, float *qw){
  uint8_t arrLength = 16;
  byte buff[arrLength];
  float arr[arrLength / 4]; // Array to store the extracted and converted values
  uint8_t a = arrLength - 1;
  uint8_t b = 0;
  I2Cdev::readBytes(devAddr, SentralMM_RESULT_QX_LL, arrLength, buff);
  while(a >= 0)
  {
    uint32_t val = (buff[a] << 24) | (buff[a - 1] << 16) | (buff[a - 2] << 8) | buff[a - 3];
    arr[b++] = *(float*)&val; // Converts IEEE 754 32 bit to float
    a = a - 4;
  }
  *qx = arr[0];
  *qy = arr[1];
  *qz = arr[2];
  *qw = arr[3];
}

/*Gets the quaternion values from the sensor
 * @param qx - The address where the X quaternion component is stored
 * @param qy - The address where the Y quaternion component is stored
 * @param qz - The address where the Z quaternion component is stored
 * @param qw - The address where the W quaternion component is stored
 */
void SentralMM::getQuat(Quaternion* q){
  uint8_t arrLength = 16;
  uint8_t buff[arrLength];
  uint32_t arr[arrLength / 4]; // Array to store the extracted and converted values
  uint8_t a = arrLength - 1;
  uint8_t b = 0;
  //Serial.println("Getting Quats");
  int8_t ret = I2Cdev::readBytes(devAddr, SentralMM_RESULT_QX_LL, arrLength, buff);
  //Serial.println("Gotten Quats");
if(ret != -1)
{
  //Serial.print("ret: ");Serial.println(ret);
  arr[0] = ((buff[3] << 24) | (buff[2] << 16) | (buff[1] << 8) | buff[0]);
  arr[1] = ((buff[7] << 24) | (buff[6] << 16) | (buff[5] << 8) | buff[4]);
  arr[2] = ((buff[11] << 24) | (buff[10] << 16) | (buff[9] << 8) | buff[8]);
  arr[3] = ((buff[15] << 24) | (buff[14] << 16) | (buff[13] << 8) | buff[12]);
}
  
//  for(; a >=0; a-=4)  {
//    Serial.print("arrLength: ");Serial.println(arrLength);
//    //uint32_t val = (buff[a] << 24) | (buff[a - 1] << 16) | (buff[a - 2] << 8) | buff[a - 3];
//    //arr[b++] = *(float*)&val; // Converts IEEE 754 32 bit to float
////    a = a - 4;
//    //Serial.println("arrLength: ");Serial.println(arrLength);
//    
//  }

  //Serial.print("abuff[1]: ");Serial.println(buff[1]);
  //Serial.print("*(float*)&(arr[0]): ");Serial.println(*(float*)&(arr[0]));
  q -> x = *(float*)&(arr[0]);
  q -> y = *(float*)&(arr[1]);
  q -> z = *(float*)&(arr[2]);
  q -> w = *(float*)&(arr[3]);
}

/*Gets the quaternion values and quaternion read-time from the sensor
 * @param qx - The address where the X quaternion component is stored
 * @param qy - The address where the Y quaternion component is stored
 * @param qz - The address where the Z quaternion component is stored
 * @param qw - The address where the W quaternion component is stored
 * @param qTime - The address where the quaternion time is stored
 */
void SentralMM::getQuat(float* qx, float* qy, float* qz, float *qw, uint16_t* qTime){
  getQuat(qx, qy, qz, qw);
  *qTime = getQuatTime();
}

float SentralMM::getQuatX(){
  uint8_t arrLength = 4;
  byte buff[arrLength];
  I2Cdev::readBytes(devAddr, SentralMM_RESULT_QX_LL, arrLength, buff);
  uint32_t val = (buff[3] << 24) | (buff[2] << 16) | (buff[1] << 8) | buff[0];
  return *(float*)&val; // Converts IEEE 754 32 bit to float
}

float SentralMM::getQuatY(){
  uint8_t arrLength = 4;
  byte buff[arrLength];
  I2Cdev::readBytes(devAddr, SentralMM_RESULT_QY_LL, arrLength, buff);
  uint32_t val = (buff[3] << 24) | (buff[2] << 16) | (buff[1] << 8) | buff[0];
  return *(float*)&val; // Converts IEEE 754 32 bit to float
}

float SentralMM::getQuatZ(){
  uint8_t arrLength = 4;
  byte buff[arrLength];
  I2Cdev::readBytes(devAddr, SentralMM_RESULT_QZ_LL, arrLength, buff);
  uint32_t val = (buff[3] << 24) | (buff[2] << 16) | (buff[1] << 8) | buff[0];
  return *(float*)&val; // Converts IEEE 754 32 bit to float
}

float SentralMM::getQuatW(){
  uint8_t arrLength = 4;
  byte buff[arrLength];
  I2Cdev::readBytes(devAddr, SentralMM_RESULT_QW_LL, arrLength, buff);
  uint32_t val = (buff[3] << 24) | (buff[2] << 16) | (buff[1] << 8) | buff[0];
  return *(float*)&val; // Converts IEEE 754 32 bit to float
}

uint16_t SentralMM::getQuatTime(){
   I2Cdev::readBytes(devAddr, SentralMM_RESULT_QTIME_LL, 2, buffer);
   return (buffer[1] << 8) | buffer[0];
}

uint8_t SentralMM::quat2YPR(float *data, Quaternion *q) {
    // yaw: (about Z axis)
    data[0] = atan2(2.0 * (q->y * q->z + q->w * q->x), (q->w * q->w) + (q->x * q->x) - (q->y * q->y) - (q->z * q->z));
    // pitch: (nose up/down, about Y axis)
    data[1] = asin(-2.0 * (q->x * q->z - q->w*q->y));
    // roll: (tilt left/right, about X axis)
    data[2] = atan2(2.0 * (q->x * q->y + q->w * q->z), (q->w * q->w) - (q->x * q->x) - (q->y * q->y) + (q->z * q->z));
    return 0;
}

