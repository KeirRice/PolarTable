/*************************************************************
  Manage wifi
*************************************************************/

#include "password.h"
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
  
  // wifiManager.setDebugOutput(false);
  
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);
  
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setAPCallback(configModeCallback);
  
  while(!wifiManager.autoConnect("PolarTableConfig", PASSWORD)){
    delay(300000); // 5 min retry loop
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
