//Communicate between ESP8266
//Send over to firebase

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

#define FIREBASE_HOST "ultima-apparel.firebaseio.com" //"al-test-916f1.firebaseio.com"
#define FIREBASE_AUTH "0rTZmBQf86XpBEkF0rtG5KqjAwq8WSbevI1WvBok"
#define WIFI_SSID "Jo_V30"
#define WIFI_PASSWORD "itspassword"

//#define

String dataPath = "/users/data/-L5ohOlG020TA2K3tXrg/";
String timeStampPath = "/users/assignments/-L5ohOlG020TA2K3tXrg/currentTime";

String databaseTime = "";
unsigned long startTime = 0;



void setup() {
  Serial.begin(9600);
  delay(3000);

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

  Firebase.begin(FIREBASE_HOST, ""); // FIREBASE_AUTH);


  StaticJsonBuffer<50> timeStampBuffer;
  JsonObject& timeStampObject = timeStampBuffer.createObject();
  timeStampObject[".sv"] = "timestamp";

  // Updates the Date-Time Stamp on firebase to timeStampPath
  Firebase.set(timeStampPath, timeStampObject);
  if (Firebase.failed())
  {
    Serial.print("setting /number failed: ");
    Serial.println(Firebase.error());
    return;
  } else {
    Serial.println("Uploaded");
  }


  // Retrieve the Date-Time Stamp on firebase from timeStampPath
  FirebaseObject fbj = Firebase.get(timeStampPath); //Using the Firebase.get function, a FirebaseObject value is returned (similar to stock Firebase's "Snapshot")
  if (Firebase.failed()) // Checks if operation failed
  {
    Serial.print("Error Getting Object: ");
    Serial.println(Firebase.error());
    return;
  } else {
    Serial.println("Retrieved Timestamp!");
  }

  JsonObject& timeStampObj = timeStampBuffer.createObject();
  timeStampObj["timestamp"] = fbj.getJsonVariant();
  Serial.print("TimeStamp = "); timeStampObj["timestamp"].printTo(Serial);Serial.println();
  databaseTime = timeStampObj.get<String>("timestamp");
  
  String temp = databaseTime.substring(3);
  Serial.print("temp = ");Serial.println(temp);
  char charStr[50] ;
  temp.toCharArray(charStr,sizeof(charStr));
  Serial.print("charStr = ");Serial.println(charStr);
  startTime = strtoul (charStr, NULL, 0); //COnverts string to unsigned Long
  Serial.print("startTime = ");Serial.println(startTime);
  
  

}//############# Setup Ends Here ####################



StaticJsonBuffer<512> arrBuffer; // Timestamp Level
StaticJsonBuffer<256> sensorBuff; // used to create data Json objects

//array.printTo(Serial);

int quatNum = 0;
int sensorNum = 0;
long timeStamp = 0;
String sensorID[7];
float quats[7][4];

String str = "";
char chr;


void loop()
{
//  if (sensorNum == 7)
//  {
//    JsonArray& arr = arrBuffer.createArray(); // SensorId level
//    for (int i = 0; i < sensorNum; i++)
//    {
//      JsonObject& root = sensorBuff.createObject();
//      JsonObject& nested = root.createNestedObject(sensorID[i]); // ... => sensorObj
//      nested["qw"] = quats[i][0];
//      nested["qx"] = quats[i][1];
//      nested["qy"] = quats[i][2];
//      nested["qz"] = quats[i][3];
//
//      arr.add(root);
//    }
//
//    String newDataPath = dataPath + (timeStamp + startTime);
//    Firebase.set(, arr); //arrBuffer.parse(arr));
//    if (Firebase.failed())
//    {
//      Serial.print("Data Transmission Failed: ");
//      Serial.println(Firebase.error());
//      return;
//    }
//    sensorNum = 0;
//  }
//
//  if (Serial.available() > 0)
//  {
//    chr = Serial.read();
//    if (chr == 'A')
//    {
//      sensorNum = 1;
//      sensorID[sensorNum - 1] = chr;
//      quatNum = 0;
//      str = "";
//    }
//    else if (chr == 'B')
//    {
//      sensorNum = 2;
//      sensorID[sensorNum - 1] = chr;
//      quatNum = 0;
//      str = "";
//    }
//    else if (chr == 'C')
//    {
//      sensorNum = 3;
//      sensorID[sensorNum - 1] = chr;
//      quatNum = 0;
//      str = "";
//    }
//    else if (chr == 'D')
//    {
//      sensorNum = 4;
//      sensorID[sensorNum - 1] = chr;
//      quatNum = 0;
//      str = "";
//    }
//    else if (chr == 'E')
//    {
//      sensorNum = 5;
//      sensorID[sensorNum - 1] = chr;
//      quatNum = 0;
//      str = "";
//    }
//    else if (chr == 'F')
//    {
//      sensorNum = 6;
//      sensorID[sensorNum - 1] = chr;
//      quatNum = 0;
//      str = "";
//    }
//    else if (chr == 'G')
//    {
//      sensorNum = 7;
//      sensorID[sensorNum - 1] = chr;
//      quatNum = 0;
//      str = "";
//    }
//    else if (chr == ',')
//    {
//      quats[sensorNum - 1][quatNum] = str.toFloat();
//      quatNum++;
//      str = "";
//
//    }
//    else if (chr == '|')
//    {
//      char conv[256];
//      str.toCharArray(conv, 256);
//      timeStamp = atol(conv);
//      str = "";
//    }
//    else if (chr == '\n' || chr == '\r')
//    {
//
//    }
//    else
//    {
//      str += chr;
//    }
//
//
//
//  }
}
