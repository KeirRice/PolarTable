#include <ESP8266WiFi.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

#include <EEPROM.h>
#include <string.h>

const char* ssid = "paprika2"; //SSID STATION MODE
const char* password = "A11AFGL409398"; //Password STATION MODE

const char* myssid = "PolarTable"; //SSID AP MODE
const char* mypassword = "1234"; //Password AP MODE

WiFiServer server(80);

ESP8266WebServer serverAP(8000);

long timeout = 0;
char estado = 0;

#define OUT1 5

void handleRoot()
{
  serverAP.send(200, "text/html", "<h1>You are connected</h1>");
}

void setup()
{
//  pinMode(OUT1, OUTPUT);
//  digitalWrite(OUT1, 0);

  Serial.begin(115200);
  Serial.print("");
  
  WiFi.mode(WIFI_AP_STA);

  Serial.print("Bringing up AP");
  //AP
  Serial.println(WiFi.softAP(myssid, mypassword) ? "Ready" : "Failed!");
  
  serverAP.on("/", handleRoot);
  serverAP.begin();

//  //Station
//  Serial.print("Connecting to paprika");
//  WiFi.begin(ssid, password);
//
//  while (WiFi.status() != WL_CONNECTED)
//  {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println();
//
//  Serial.print("Connected, IP address: ");
//  Serial.println(WiFi.localIP());
  
}

void loop()
{
  serverAP.handleClient();

  if (estado == 0)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      server.begin();
      server.setNoDelay(true);
      estado = 1;
    }
  }

//  if (estado == 1)
//  {
//    WiFiClient client = server.available();
//
//    if (!client)
//    {
//      return;
//    }
//
//    timeout = 0;
//    while (!client.available() && timeout <= 1000)
//    {
//      delay(1);
//      timeout++;
//    }
//
//    String req = client.readStringUntil('\r');
//    client.flush();
//
//    String resposta = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n ";
//
//    if (req.indexOf("/A") != -1)
//    {
//      digitalWrite(OUT1, 1);
//      resposta += "OUT1 ON";
//    }
//    else if (req.indexOf("/a") != -1)
//    {
//      digitalWrite(OUT1, 0);
//      resposta += "OUT1 OFF";
//    }
//
//    client.flush();
//    resposta += "</html>\n";
//    client.print(resposta);
//    delay(1);
//  }
}

//#include <ESP8266WiFi.h>
//
//void setup()
//{
//  Serial.begin(115200);
//  Serial.println();
//
//  WiFi.begin("paprika2", "A11AFGL409398");
//
//  Serial.print("Connecting");
//  while (WiFi.status() != WL_CONNECTED)
//  {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println();
//
//  Serial.print("Connected, IP address: ");
//  Serial.println(WiFi.localIP());
//}
//
//void loop() {}
