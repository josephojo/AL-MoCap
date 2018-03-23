
/*
 *  3 - 02-10-2018 - 
 *  2 - 02-10-2018 - Adapted Jeff Rowberg's MPU6050d library for the Sentral_MM (J.Ojo)
 *  1 - 06-01-2017 - Added a couple of methods, all of which as commented with "J.Ojo"
 *  0 - XX-XX-XXXX - Original Code by Jeff Rowberg
 *  # - DD-MM-YYYY - "Comments..."
 * --------------REVISION LOG---------------
 */

 /* NOTES:
  *       Comments having brackets "[]" are keywords that can be found in the Sentral M&M module Datasheet
  * 
  * 
  * 
  */

#ifndef _SentralMM_H_
#define _SentralMM_H_

//#include "i2c_t3.h"
#include "I2Cdev.h"
#include "helper_3dmath.h"


#define SentralMM_ADDRESS_AD0_LOW     0x28 //default for Sentral M&M Blue & Teal
#define SentralMM_DEFAULT_ADDRESS     SentralMM_ADDRESS_AD0_LOW

// --- Sentral Status (Read Only) ---
#define SentralMM_SSTATUS               0x37 // Register for checking the status of the configuration file upload from the EEPROM [SentralStatus](SS)
#define SentralMM_SS_EE_DET_BIT            0 // EEPROM detection bit. 1 = the EEPROM has been detected
#define SentralMM_SS_EE_UPLOADED_BIT       1 // Configuration file uploaded bit. 1 = File upload Done 
#define SentralMM_SS_EE_UPLOAD_ERR_BIT     2 // 1 = Calculated CRC of EEPROM is incorrect. Only valid when EEUploadDone = 1 .
#define SentralMM_SS_EE_UPLOAD_IDLE_BIT    3 // 1 = Device in Unprogrammed or Initialized state.
#define SentralMM_SS_NO_EE_DET_BIT         4 // 1 = No EEPROM detected.
#define SentralMM_SS_LENGTH                3 // Number of bits we need from the sentral status register

// --- Sentral Reset (Write Only) ---
#define SentralMM_RESETREQ      0x9B  // Register for making a reset request or force restart
#define SentralMM_RESETREQ_BIT     0 // 1 = Emulate a hard power down/power up

// --- Sentral Initial Setup ---
#define SentralMM_MAGRATE         0x55  // Requested magnetometer output data rate
#define SentralMM_ACCELRATE       0x56  // Requested accelerometer output data rate divided by 10
#define SentralMM_GYRORATE        0X57  // Requested gyroscope output data rate divided by 10
#define SentralMM_QRATE_DIVISOR   0x32  // Along with GyroRate, establishes output data rate for quaternion data.

// --- (Write Only)---
#define SentralMM_ALG_CTRL                0x54  // Register for controling what algorithm runs and calculates the data [AlgorithmControl]
#define SentralMM_ALG_CTRL_STANDBY_BIT       0  // 1 = StandbyEnable, 0 = Disable Standby State (return to Normal Operation)
#define SentralMM_ALG_CTRL_RAWDATA_BIT       1  // RawDataEnable. 1 = Raw data provided in MX, MY,MZ, AX, AY, AZ, GX, GY, & GZ. 0 = Scaled sensor data.
#define SentralMM_ALG_CTRL_HPR_QUAT_BIT      2  // HPRoutput. 1 = Heading, pitch, and roll output in QX, QY, & QZ. QW = 0.0. 0 = Quaternion outputs.

// --- (Write Only)---
#define SentralMM_EN_EVENT_INTERRUPT         0x33  // "1‟ indicates an interrupt to the host will be generated for the event.
#define SentralMM_EN_EVENT_INTERRUPT_CPU_RST    0  // Non-Maskable
#define SentralMM_EN_EVENT_INTERRUPT_ERR_BIT    1  // 1 = Error Occured
#define SentralMM_EN_EVENT_INTERRUPT_QUAT_BIT   2  // 1 = Quaternion Result is available
#define SentralMM_EN_EVENT_INTERRUPT_MAG_BIT    3  // 1 = Magnetometer Result is available
#define SentralMM_EN_EVENT_INTERRUPT_ACCL_BIT   4  // 1 = Accelerometer Result is available 
#define SentralMM_EN_EVENT_INTERRUPT_GYRO_BIT   5  // 1 = Gyroscope Result is available
#define SentralMM_EN_EVENT_LENGTH               5  // Number of bits we are using from the EN event register

// --- Normal Operation Status Registers (Read and Write)---
#define SentralMM_HOST_CONTROL      0x34  // [0] 1 = RunEnable, 0 = Enable Initialized State (Standby State generally is preferred since enabling Initialized State resets the
                                          // SENtral algorithm, including calibration data.)
// --- (Read Only) ---
#define SentralMM_INTERRUPT_STATUS       0x35  // "1‟ indicates an interrupt to the host will be generated for the event.
#define SentralMM_INTERRUPT_STATUS_CPU_RST  0  // Non-Maskable
#define SentralMM_INTERRUPT_STATUS_ERR_BIT  1  // 1 = Error Occured
#define SentralMM_INTERRUPT_STATUS_QUAT_BIT 2  // 1 = Quaternion Result is available
#define SentralMM_INTERRUPT_STATUS_MAG_BIT  3  // 1 = Magnetometer Result is available
#define SentralMM_INTERRUPT_STATUS_ACCL_BIT 4  // 1 = Accelerometer Result is available 
#define SentralMM_INTERRUPT_STATUS_GYRO_BIT 5  // 1 = Gyroscope Result is availabl
#define SentralMM_INT_STATUS_LENGTH         6  // Number of bits we are using from the EN event reg

#define SentralMM_ALG_STATUS        0x38  // [0] 1 = SENtral in Standby State, 0 = SENtral not in Standby State 

/* 
 *  Result Registers
 */

// --- Quaternion Results ---
#define SentralMM_RESULT_QX_LL    0x00  // LSB     -> Normalized Quaternion – X, or Heading - Float32 - 0.0 – 1.0, or ±PI
#define SentralMM_RESULT_QX_LH    0x01  // LSB + 1 -> Normalized Quaternion – X, or Heading - Float32 - 0.0 – 1.0, or ±PI
#define SentralMM_RESULT_QX_HL    0x02  // MSB - 1 -> Normalized Quaternion – X, or Heading - Float32 - 0.0 – 1.0, or ±PI
#define SentralMM_RESULT_QX_HH    0x03  // MSB     -> Normalized Quaternion – X, or Heading - Float32 - 0.0 – 1.0, or ±PI

#define SentralMM_RESULT_QY_LL    0x04  // LSB     -> Normalized Quaternion – Y, or Pitch - Float32 - 0.0 – 1.0, or ±PI/2
#define SentralMM_RESULT_QY_LH    0x05  // LSB + 1 -> Normalized Quaternion – Y, or Pitch - Float32 - 0.0 – 1.0, or ±PI/2
#define SentralMM_RESULT_QY_HL    0x06  // MSB - 1 -> Normalized Quaternion – Y, or Pitch - Float32 - 0.0 – 1.0, or ±PI/2
#define SentralMM_RESULT_QY_HH    0x07  // MSB     -> Normalized Quaternion – Y, or Pitch - Float32 - 0.0 – 1.0, or ±PI/2

#define SentralMM_RESULT_QZ_LL    0x08  // LSB     -> Normalized Quaternion – Z, or Roll - Float32 - 0.0 – 1.0, or ±PI
#define SentralMM_RESULT_QZ_LH    0x09  // LSB + 1 -> Normalized Quaternion – Z, or Roll - Float32 - 0.0 – 1.0, or ±PI
#define SentralMM_RESULT_QZ_HL    0x0A  // MSB - 1 -> Normalized Quaternion – Z, or Roll - Float32 - 0.0 – 1.0, or ±PI
#define SentralMM_RESULT_QZ_HH    0x0B  // MSB     -> Normalized Quaternion – Z, or Roll - Float32 - 0.0 – 1.0, or ±PI

#define SentralMM_RESULT_QW_LL    0x0C  // LSB     -> Normalized Quaternion – W, or 0.0 - Float32 - 0.0 – 1.0
#define SentralMM_RESULT_QW_LH    0x0D  // LSB + 1 -> Normalized Quaternion – W, or 0.0 - Float32 - 0.0 – 1.0
#define SentralMM_RESULT_QW_HL    0x0E  // MSB - 1 -> Normalized Quaternion – W, or 0.0 - Float32 - 0.0 – 1.0
#define SentralMM_RESULT_QW_HH    0x0F  // MSB     -> Normalized Quaternion – W, or 0.0 - Float32 - 0.0 – 1.0

#define SentralMM_RESULT_QTIME_LL 0x10  // LSB     -> Quaternion Data Timestamp - UInt16 - 0 – 2048 msec
#define SentralMM_RESULT_QTIME_HH 0x11  // MSB     -> Quaternion Data Timestamp - UInt16 - 0 – 2048 msec

// --- Magnetometer Results ---
#define SentralMM_RESULT_MX_LL    0x12  // LSB    -> Magnetic Field – X Axis, or Raw Mag Data - Int16 - ±1000 µT when scaled
#define SentralMM_RESULT_MX_HH    0x13  // MSB    -> Magnetic Field – X Axis, or Raw Mag Data - Int16 - ±1000 µT when scaled

#define SentralMM_RESULT_MX_LL    0x14  // LSB    -> Magnetic Field – Y Axis, or Raw Mag Data - Int16 - ±1000 µT when scaled
#define SentralMM_RESULT_MX_HH    0x15  // MSB    -> Magnetic Field – Y Axis, or Raw Mag Data - Int16 - ±1000 µT when scaled

#define SentralMM_RESULT_MX_LL    0x16  // LSB    -> Magnetic Field – Z Axis, or Raw Mag Data - Int16 - ±1000 µT when scaled
#define SentralMM_RESULT_MX_HH    0x17  // MSB    -> Magnetic Field – Z Axis, or Raw Mag Data - Int16 - ±1000 µT when scaled

#define SentralMM_RESULT_MTIME_LL 0x18  // LSB     -> Magnetometer Interrupt Timestamp - UInt16 - 0 – 2048 msec
#define SentralMM_RESULT_MTIME_HH 0x19  // MSB     -> Magnetometer Interrupt Timestamp - UInt16 - 0 – 2048 msec

// --- Accelerometer Results ---
#define SentralMM_RESULT_AX_LL    0x1A  // LSB    -> Linear Acceleration – X Axis, or Raw Accel Data - Int16 - ±16 g when scaled
#define SentralMM_RESULT_AX_HH    0x1B  // MSB    -> Linear Acceleration – X Axis, or Raw Accel Data - Int16 - ±16 g when scaled

#define SentralMM_RESULT_AX_LL    0x1C  // LSB    -> Linear Acceleration – Y Axis, or Raw Accel Data - Int16 - ±16 g when scaled
#define SentralMM_RESULT_AX_HH    0x1D  // MSB    -> Linear Acceleration – Y Axis, or Raw Accel Data - Int16 - ±16 g when scaled

#define SentralMM_RESULT_AX_LL    0x1E  // LSB    -> Linear Acceleration – Z Axis, or Raw Accel Data - Int16 - ±16 g when scaled
#define SentralMM_RESULT_AX_HH    0x1F  // MSB    -> Linear Acceleration – Z Axis, or Raw Accel Data - Int16 - ±16 g when scaled

#define SentralMM_RESULT_ATIME_LL 0x20  // LSB     -> Accelerometer Interrupt Timestamp - UInt16 - 0 – 2048 msec
#define SentralMM_RESULT_ATIME_HH 0x21  // MSB     -> Accelerometer Interrupt Timestamp - UInt16 - 0 – 2048 msec

// --- Gyroscope Results ---
#define SentralMM_RESULT_GX_LL    0x22  // LSB    -> Rotational Velocity – X Axis, or Raw Gyro Data - Int16 - ±5000°/s when scaled
#define SentralMM_RESULT_GX_HH    0x23  // MSB    -> Rotational Velocity – X Axis, or Raw Gyro Data - Int16 - ±5000°/s when scaled

#define SentralMM_RESULT_GX_LL    0x24  // LSB    -> Rotational Velocity – X Axis, or Raw Gyro Data - Int16 - ±5000°/s when scaled
#define SentralMM_RESULT_GX_HH    0x25  // MSB    -> Rotational Velocity – X Axis, or Raw Gyro Data - Int16 - ±5000°/s when scaled

#define SentralMM_RESULT_GX_LL    0x26  // LSB    -> Rotational Velocity – X Axis, or Raw Gyro Data - Int16 - ±5000°/s when scaled
#define SentralMM_RESULT_GX_HH    0x27  // MSB    -> Rotational Velocity – X Axis, or Raw Gyro Data - Int16 - ±5000°/s when scaled

#define SentralMM_RESULT_GTIME_LL 0x28  // LSB     -> Gyroscope Interrupt Timestamp - UInt16 - 0 – 2048 msec
#define SentralMM_RESULT_GTIME_HH 0x29  // MSB     -> Gyroscope Interrupt Timestamp - UInt16 - 0 – 2048 msec

// --- Pass-Through Registers ---

#define SentralMM_PASS_THRU_CTRL    0xA0  // [0] 1 = Enable Pass-Through State, 0 = Disable Pass-Through State
#define SentralMM_PASS_THRU_STATUS  0x9E  // [0] 1 = Sentral is in Pass-Through State, 0 = Sentral is not in Pass-Through State



// note: DMP code memory blocks defined at end of header file

class SentralMM {
    public:
  
        SentralMM();
        //SentralMM(uint8_t address);

        uint8_t initialize(uint16_t timeout = 1000);
        bool testConnection();
        uint8_t getSentralStatus();
//        uint8_t getEEPROM();
        uint8_t getEEUploadDone();
        uint8_t getEEUploadErr();

        // Data Rates - Gyro and Accel rates needed should be divided by 10. 
        // E.g for 200Hz, input is 20.
        // QRate = GRate / QrateDivisor

        void setSensorRates(uint8_t gRate, uint8_t aRate, uint8_t mRate);
        uint8_t getQRateDivisor();
        void setQRateDivisor(uint8_t divisor);
        uint8_t getMRate();
        void setMRate(uint8_t rate);
        uint8_t getARate();
        void setARate(uint8_t rate);
        uint8_t getGRate();
        void setGRate(uint8_t rate);

       
        // INT_ENABLE register
        void setIntEvents(uint8_t val);
        uint8_t getIntEvents();
        void setErrorIntEvent(bool enabled);
        uint8_t getErrorIntEvent();
        void setQuatResultIntEvent(bool enabled);
        uint8_t getQuatResultIntEvent();
        void setMagResultIntEvent(bool enabled);
        uint8_t getMagResultIntEvent();
        void setAccelResultIntEvent(bool enabled);
        uint8_t getAccelResultIntEvent();
        void setGyroResultIntEvent(bool enabled);
        uint8_t getGyroResultIntEvent();
        uint8_t getIntStatus();
        uint8_t getResetStatus();


     // --- Sensor Control ---

        // Reset Sentral (similar to reboot)
        bool restartSentral();

        // Algorithm Control
        void setAlgControl(uint8_t alg);
        void setRawDataType(uint8_t val);
        void setAngleType(uint8_t val);
        
    // --- Set Sensor State ---
        // Pass thru is when the host is speaking directly with individual sensors
        void setPassThruState(bool enabled);
        bool getPassThruState();
        
        // Stanby State configurations
        void setStandbyState(bool enabled);
        bool getStandbyState();
        bool getStandbyStatus();

        // Normal Operation State Configuration
        void setNormalState(bool enabled);
        bool getNormalState();
    // ------------------------
    
        // Get Algorithm status
        bool getAlgStatus();
        bool getPassThruStatus();

        // ------ Data Collection from Sensor --------
        void getSensorBytes(uint8_t* data, uint8_t count); // Not Implemented
        void getRawData9(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz, int16_t* mx, int16_t* my, int16_t* mz); // Not implemented

        // Converts  byte to float32 using IEEE 754. Assuming Little Endian
        float byte2float32(uint8_t* data);
        
        // Quaternion_*OUT_* registers
        void getQuat(float* qx, float* qy, float* qz, float *qw);
        void getQuat(Quaternion* q);
        void getQuat(float* qx, float* qy, float* qz, float *qw, uint16_t* qTime);
        float getQuatX();
        float getQuatY();
        float getQuatZ();
        float getQuatW();
        uint16_t getQuatTime(); 
        
        // ACCEL_*OUT_* registers
        void getAccel(int16_t* ax, int16_t* ay, int16_t* az);
        void getAccel(int16_t* ax, int16_t* ay, int16_t* az, uint16_t* aTime);
        int16_t getAccelX();
        int16_t getAccelY();
        int16_t getAccelZ();
        uint16_t getAccelTime();

        // GYRO_*OUT_* registers
        void getGyro(int16_t* gx, int16_t* gy, int16_t* gz);
        void getGyro(int16_t* gx, int16_t* gy, int16_t* gz, uint16_t* gTime);
        int16_t getGyroX();
        int16_t getGyroY();
        int16_t getGyroZ();
        uint16_t getGyroTime();

        // MAG_*OUT_* registers
        void getMag(int16_t* mx, int16_t* my, int16_t* mz);
        void getMag(int16_t* mx, int16_t* my, int16_t* mz, uint16_t* mTime);
        int16_t getMagX();
        int16_t getMagY();
        int16_t getMagZ();
        uint16_t getMagTime();

        uint8_t quat2YPR(float *data, Quaternion *q);
        
//        
//            // Get Fixed Point data from FIFO
//            uint8_t extractAccel(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractAccel(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractAccel(VectorInt16 *v, const uint8_t* packet=0);
//            uint8_t extractQuaternion(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractQuaternion(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractQuaternion(Quaternion *q, const uint8_t* packet=0);
//            uint8_t extract6AxisQuaternion(int32_t *data, const uint8_t* packet=0);
//            uint8_t extract6AxisQuaternion(int16_t *data, const uint8_t* packet=0);
//            uint8_t extract6AxisQuaternion(Quaternion *q, const uint8_t* packet=0);
//            uint8_t extractRelativeQuaternion(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractRelativeQuaternion(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractRelativeQuaternion(Quaternion *data, const uint8_t* packet=0);
//            uint8_t extractGyro(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractGyro(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractGyro(VectorInt16 *v, const uint8_t* packet=0);
//            uint8_t dmpSetLinearAccelFilterCoefficient(float coef);
//            uint8_t extractLinearAccel(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractLinearAccel(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractLinearAccel(VectorInt16 *v, const uint8_t* packet=0);
//            uint8_t extractLinearAccel(VectorInt16 *v, VectorInt16 *vRaw, VectorFloat *gravity);
//            uint8_t extractLinearAccelInWorld(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractLinearAccelInWorld(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractLinearAccelInWorld(VectorInt16 *v, const uint8_t* packet=0);
//            uint8_t extractLinearAccelInWorld(VectorInt16 *v, VectorInt16 *vReal, Quaternion *q);
//            uint8_t extractGyroAndAccelSensor(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractGyroAndAccelSensor(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractGyroAndAccelSensor(VectorInt16 *g, VectorInt16 *a, const uint8_t* packet=0);
//            uint8_t extractGyroSensor(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractGyroSensor(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractGyroSensor(VectorInt16 *v, const uint8_t* packet=0);
//            uint8_t extractControlData(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractTemperature(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractGravity(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractGravity(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractGravity(VectorInt16 *v, const uint8_t* packet=0);
//            uint8_t extractGravity(VectorFloat *v, Quaternion *q);
//            uint8_t extractUnquantizedAccel(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractUnquantizedAccel(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractUnquantizedAccel(VectorInt16 *v, const uint8_t* packet=0);
//            uint8_t extractQuantizedAccel(int32_t *data, const uint8_t* packet=0);
//            uint8_t extractQuantizedAccel(int16_t *data, const uint8_t* packet=0);
//            uint8_t extractQuantizedAccel(VectorInt16 *v, const uint8_t* packet=0);
//            uint8_t extractEIS(int32_t *data, const uint8_t* packet=0);
            
//            uint8_t extractEuler(float *data, Quaternion *q);
//            uint8_t extractYawPitchRoll(float *data, Quaternion *q, VectorFloat *gravity);
//
//            // Get Floating Point data from FIFO
//            uint8_t extractAccelFloat(float *data, const uint8_t* packet=0);
//            uint8_t extractQuaternionFloat(float *data, const uint8_t* packet=0);

    private:
        uint8_t devAddr;
        uint8_t buffer[14];
};

#endif /* _MPU6050_H_ */
