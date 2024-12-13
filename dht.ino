//---thingspeak ---- ////

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include "DHT.h"  
 DHT dht2(2,DHT11);  


////////////----------Thingspeak---------////////////

const char* ssid = "Guest";
const char* password = "888joy888";

const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "70BWCN9QEST0T0ST";

unsigned long lastTime = 0;

unsigned long timerDelay = 10000;

/////////// ---------thingspeak-------//////////////////////



 void setup()  
 {  
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  
 } 

 }

void loop()
{
  
   Serial.println("Temperature in C:");  
   Serial.println((dht2.readTemperature( )+20));  
  
   delay(1000);  
   
   /////////// ---------thingspeak-------//////////////////////
   if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "api_key=" + apiKey + "&field2=" + String(dht2.readTemperature()+20);           
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);

Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }

//////////// ----- thingsspeak -----/////////


}
