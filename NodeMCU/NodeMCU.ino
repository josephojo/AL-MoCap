//Communicate between ESP8266
//Send over to firebase 

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

// Set these to run example.
#define FIREBASE_HOST "capstone-93530.firebaseio.com"
#define FIREBASE_AUTH "xCTPZkhkxeJmjUI6HGAYSEFnpnJNR4jOn5SsqBZ7"
#define WIFI_SSID "lg"
#define WIFI_PASSWORD "itspassword"

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
int numQuats = 0;
int numSensors = 0;
int numDatashot = 0;
String str = "";
void loop() {
    while(Serial.available()> 0) {
      //inputString = Serial.read();
      char chr = Serial.read();
      if(chr == 'A' || chr == 'B' || chr == 'C' || chr == 'D' || chr == 'E' || chr == 'F')
      {
        Firebase.pushString("Sensor/" + (String)chr + "", "Null");
        delay(2000);
      }
      else if(chr == ',')
      {
        numQuats++;
        // Store str.toFloat into a float variable
      }
      else if(chr == '/')
      {
        
      }
      else if(chr == '|')
      {
        
      }
      else if(chr == '\n')
      {
        
      }
      else
      {
        str += chr;
      }
  } 
   if(numDatashot != 0)
  {  
  
  }
  //   Firebase.setString("Input Text", inputString);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());  
      return;
  } 
}
