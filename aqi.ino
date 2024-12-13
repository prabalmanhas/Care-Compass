#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#define buzzer D4


//Wifi Cedentials
const char* ssid = "Tiwary";
const char* password = "12345678";
//Thingspeak


String apiKey = "9QIOPGH1A8WZZJ6Y"; // API key

const char* server = "http://api.thingspeak.com/update";

unsigned long lastTime = 0;

unsigned long timerDelay = 10000;

/////////// ---------thingspeak-------//////////////////////



 void setup()  
 {  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  
 } 

 }



WiFiClient client;


float Level;

void loop()

{

float reading = analogRead(A0);

Level = ((reading/1023)*100);

if (isnan(Level))

{

Serial.println("Failed to read from MQ-2 sensor!");

}

 /////////// ---------thingspeak-------//////////////////////
 if(Level>50)
 {
   digitalWrite(buzzer, HIGH);
  delay(200);
   digitalWrite(buzzer, LOW);
  delay(200); 
 }
   if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, server);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "api_key=" + apiKey + "&field1=" + String(Level);           
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
}

//////////// ----- thingsspeak -----/////////
