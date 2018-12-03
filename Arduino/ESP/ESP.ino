/*************************************************************
  Manage wifi
*************************************************************/

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  Serial.begin(115200);
     
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(120);
  wifiManager.setDebugOutput(false);
  wifiManager.setMinimumSignalQuality(20);
  WiFi.hostname("PolarTableController");
  
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setAPCallback(configModeCallback);

  //set custom ip for portal
  wifiManager.setAPStaticIPConfig(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));

  // wifiManager.setCustomHeadElement("<style>html{filter: invert(100%); -webkit-filter: invert(100%);}</style>");
  // WiFiManagerParameter custom_text("<p>This is just a text paragraph</p>");
  // wifiManager.addParameter(&custom_text);
  while(!wifiManager.autoConnect("PolarTableConfig")){
    delay(60000); // 1 min retry loop
  }

  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  while(mySerial.available() > 0) {
    // relay everything received from mySerial to Serial    
    Serial.write(mySerial.read()); 
  }
  if (Serial.available() > 0) {
    // relay everything received from Serial to mySerial
    mySerial.write(Serial.read());        
  } 
}
