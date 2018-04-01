//Communicate between ESP8266
//Send over to firebase

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

#define FIREBASE_HOST "al-test-916f1.firebaseio.com" //"ultima-apparel.firebaseio.com" //"al-test-916f1.firebaseio.com"
#define FIREBASE_AUTH "0rTZmBQf86XpBEkF0rtG5KqjAwq8WSbevI1WvBok"
#define WIFI_SSID "Jo_V30"
#define WIFI_PASSWORD "itspassword"

#define DEBUG

// #define TRANSMIT

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
    #if defined(DEBUG) || defined(TRANSMIT)
    Serial.print(".");
    #endif
    delay(500);
  }
  #ifdef DEBUG
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
  #endif
  
   #ifdef TRANSMIT
    Serial.print("/connected");
  #endif

  Firebase.begin(FIREBASE_HOST, ""); // FIREBASE_AUTH);


  StaticJsonBuffer<50> timeStampBuffer;
  JsonObject& timeStampObject = timeStampBuffer.createObject();
  timeStampObject[".sv"] = "timestamp";

  // Updates the Date-Time Stamp on firebase to timeStampPath
  Firebase.set(timeStampPath, timeStampObject);
  if (Firebase.failed())
  {
    #ifdef DEBUG
      Serial.print("Error setting DateTime: ");
      Serial.println(Firebase.error());
    #endif
    
    return;
  } else {
    #ifdef DEBUG
      Serial.println("Uploaded");
    #endif
  }


  // Retrieve the Date-Time Stamp on firebase from timeStampPath
  FirebaseObject fbj = Firebase.get(timeStampPath); //Using the Firebase.get function, a FirebaseObject value is returned (similar to stock Firebase's "Snapshot")
  if (Firebase.failed()) // Checks if operation failed
  {
    #ifdef DEBUG
      Serial.print("Error Getting Object: ");
      Serial.println(Firebase.error());
    #endif
    
    return;
  } else {
    #ifdef DEBUG
      Serial.println("Retrieved Timestamp!");
    #endif
  }

  JsonObject& timeStampObj = timeStampBuffer.createObject();
  timeStampObj["timestamp"] = fbj.getJsonVariant();
  //Serial.print("TimeStamp = "); timeStampObj["timestamp"].printTo(Serial);Serial.println();
  databaseTime = timeStampObj.get<String>("timestamp");
  
  String temp = databaseTime.substring(3);
  //Serial.print("temp = ");Serial.println(temp);
  char charStr[50] ;
  temp.toCharArray(charStr,sizeof(charStr));
  //Serial.print("charStr = ");Serial.println(charStr);
  startTime = strtoul (charStr, NULL, 0); //COnverts string to unsigned Long
  //Serial.print("startTime = ");Serial.println(startTime);
 
}
//############# Setup Ends Here ####################


StaticJsonBuffer<1024> sensorBuff; // used to create data Json objects

int quatNum = 0;
int sensorNum = 0;
long timeStamp = 0;
String sensorID[7];
float quats[7][4];

String str = "";
char chr;

int y = 0;
void loop()
{
  if (sensorNum == 7 && quatNum == 4)
  {
    JsonObject& root = sensorBuff.createObject();// SensorId level ("DateTime" : {A,B,C,D,E,F,G})
    for (int i = 0; i < sensorNum; i++)
    {
      JsonObject& nested = root.createNestedObject(sensorID[i]); // ... => sensorObj //(String)((char)(i + 65)));
      nested["qw"] = quats[i][0];
      nested["qx"] = quats[i][1];
      nested["qy"] = quats[i][2];
      nested["qz"] = quats[i][3];
    }

    unsigned long d = (timeStamp + startTime);
    String newDataPath = dataPath + databaseTime.substring(0,3) + d ;//(timeStamp + startTime);
    #ifdef DEBUG
      Serial.print("newDataPath: ");Serial.println(newDataPath);
      Serial.print("root: ");root.printTo(Serial);Serial.println();
    #endif  
    Firebase.set(newDataPath, root);
    if (Firebase.failed())
    {
      #ifdef DEBUG
        Serial.print("Data Transmission Failed: ");
        Serial.println(Firebase.error());
      #endif
      return;
    }else
    {
      #ifdef DEBUG
        Serial.println("Data Transmitted");
      #endif
    }
    sensorNum = 0;
  }

//1234|A0.1,0.2,0.3,0.4,B0.5,0.6,0.7,0.8,C0.9,1.0,1.1,1.2,D1.3,1.4,1.5,1.6,E1.7,1.8,1.9,2.0,F2.1,2.2,2.3,2.4,G2.5,2.6,2.7,2.8,
  
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
      str.toCharArray(conv, sizeof(conv));
      timeStamp = atol(conv);
      str = "";
    }
    else if (chr == '\n' || chr == '\r')
    {
      sensorNum = 0;
    }
    else
    {
      str += chr;
    }
  }
  
}
