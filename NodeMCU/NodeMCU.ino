//Communicate between ESP8266
//Send over to firebase

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

// Set these to run example.
#define FIREBASE_HOST "ultima-apparel.firebaseio.com/users/Ar07J0EG9hWlUwQvTBEeH0pvMXu2/data/-L5ohOlG020TA2K3tXrg"
#define FIREBASE_AUTH "0rTZmBQf86XpBEkF0rtG5KqjAwq8WSbevI1WvBok"
#define WIFI_SSID "lg"
#define WIFI_PASSWORD "itspassword"

String dataPath = "data/-L5ohOlG020TA2K3tXrg/";




void setup() {
  Serial.begin(9600);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

StaticJsonBuffer<512> arrBuffer; // Timestamp Level
StaticJsonBuffer<256> sensorBuff; // used to create data Json objects

//array.printTo(Serial);

int quatNum = 0;
int sensorNum = 0;
long timeStamp = 0;
String sensorID[7];
float quats[7][4];
StaticJsonBuffer<50> jsonBuffer;
JsonObject& timeStampObject = jsonBuffer.createObject();
//timeStampObject[".sv"] = "timestamp";
long startTime = timeStampObject[".sv"];

String str = "";
char chr;

void loop() 
{

  if (sensorNum == 7)
  {
    JsonArray& arr = arrBuffer.createArray(); // SensorId level
    for(int i = 0; i < sensorNum; i++)
    {
      JsonObject& root = sensorBuff.createObject();
      JsonObject& nested = root.createNestedObject(sensorID[i]); // ... => sensorObj
      nested["qw"] = quats[i][0];
      nested["qx"] = quats[i][1];
      nested["qy"] = quats[i][2];
      nested["qz"] = quats[i][3];

      arr.add(root);
    }
    Firebase.set(dataPath + (timeStamp + startTime), arr); //arrBuffer.parse(arr));
    if (Firebase.failed()) 
    {
//      while(1)
//      {
        Serial.print("setting /number failed: ");
        Serial.println(Firebase.error());
      //}
      return;
    }
    sensorNum = 0;
  }
  
  if (Serial.available() > 0)
  {
    chr = Serial.read();
    if (chr == 'A')
    {
      sensorNum = 1;
      sensorID[sensorNum - 1] = chr;
      quatNum = 0;
      str = "";
    }
    else if (chr == 'B')
    {
      sensorNum = 2;
      sensorID[sensorNum - 1] = chr;
      quatNum = 0;
      str = "";
    }
    else if (chr == 'C')
    {
      sensorNum = 3;
      sensorID[sensorNum - 1] = chr;
      quatNum = 0;
      str = "";
    }
    else if (chr == 'D')
    {
      sensorNum = 4;
      sensorID[sensorNum - 1] = chr;
      quatNum = 0;
      str = "";
    }
    else if (chr == 'E')
    {
      sensorNum = 5;
      sensorID[sensorNum - 1] = chr;
      quatNum = 0;
      str = "";
    }
    else if (chr == 'F')
    {
      sensorNum = 6;
      sensorID[sensorNum - 1] = chr;
      quatNum = 0;
      str = "";
    }
    else if (chr == 'G')
    {
      sensorNum = 7;
      sensorID[sensorNum - 1] = chr;
      quatNum = 0;
      str = "";
    }
    else if (chr == ',')
    {
      quats[sensorNum - 1][quatNum] = str.toFloat();
      quatNum++;
      str = "";
      
    }
    else if (chr == '|')
    {
      char conv[256];
      str.toCharArray(conv, 256);
      timeStamp = atol(conv);
      str = "";
    }
    else if (chr == '\n' || chr == '\r')
    {
      
    }
    else
    {
      str += chr;
    }



  }
}
