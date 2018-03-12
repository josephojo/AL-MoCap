#include <EEPROM.h>
#include "helper_3dmath.h"

#define IMU_QTY 7

int address=0;

void setup() {
  
  //Copy and Paste the contents of the file "Desktop\MoCap\initial_orien.txt"
  Quaternion q[IMU_QTY] = 
  {
    // Sensor A: {Qw, Qx, Qy, Qz} 
    // Sensor B: {Qw, Qx, Qy, Qz} ...etc
    {1,0.002,-0.002,0},
    {1,0.002,-0.001,0},
    {0.986,-0.004,-0.162,0.023},
    {0.501,0.501,0.499,-0.499},
    {0.025,0.159,-0.004,0.987},
    {0,-0.002,0,1},
    {0,-0.002,0,1}
  };

  for(int i = 0; i < IMU_QTY; i++){
    EEPROM.put(address, q[i].w); address += 4;
    EEPROM.put(address, q[i].x); address += 4;
    EEPROM.put(address, q[i].y); address += 4;
    EEPROM.put(address, q[i].z); address += 4;
  }
}

void loop() {


}
