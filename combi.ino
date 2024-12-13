#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include "MAX30105.h"
#define LED D8
#define LED2 D3
#include "heartRate.h"

MAX30105 particleSensor;
int c;

#include "DHT.h"  
 DHT dht2(2,DHT11);
 #define HOSTIFTTT "maker.ifttt.com"
#define EVENTO "Fall_detection"
#define IFTTTKEY "ykV1BWtmlkNIK--DAEs85"
//for heart
const char* ssid = "Guest";
const char* password = "888joy888";

const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "70BWCN9QEST0T0ST";

unsigned long lastTime = 0;

unsigned long timerDelay = 10000;
#define sensorDigital 2

#define sensorAnalog A0

/////////// ---------thingspeak-------//////////////////////

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
int brightness = 0;    // how bright the LED is
int fadeAmount = 5;
float beatsPerMinute;
int beatAvg;
//int a,b,c,d,e,f,g,h;

//****
//unsigned long lastTime = 0;

//unsigned long timerDelay = 10000; 
TinyGPSPlus gps;  // The TinyGPS++ object
// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;
// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D1;
const uint8_t sda = D2;
// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;
int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;
int dcount=0;
int mailcount=0;
char temp[15];
String location1;
double MaxValue = 0.35;
double MinValue = -0.35;
int count=0;
int gpscount=0;
unsigned int timer = millis();
float latitude , longitude;
int year , month , date, hour , minute , second;
static String date_str , time_str , lat_str , lng_str;
int pm;
const int RS = D0, EN = D3, d4 = D4, d5 = D5, d6 = D6, d7 = D7;   
//const char* ssid = "Guest";   // Enter the namme of your WiFi Network.
//const char* password = "888joy888";  // Enter the Password of your WiFi Network.
//char server[] = "mail.smtp2go.com";   // The SMTP Server
//WiFiClient espClient;
 WiFiClient client;
void setup() {
  // setup for dht
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  
    //delay(500);
    Serial.print(".");

 ///------ ip address if need -----///
/*
Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");

  // Random seed is a number used to initialize a pseudorandom number generator
  randomSeed(analogRead(0));
*/


  Serial.println("Initializing... Heart Sensor");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30102 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
 } 

}

void loop() {
  setup();
  //analogWrite(LED2, brightness);
  alcohol();
  heart_sensor();
  dht();
  gyro(); 
  
  dht();

}
void dht(){

  Serial.println("Temperature in C:");  
  Serial.println((dht2.readTemperature( )+20));  
  
   //delay(1000);  
   
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
      String httpRequestData2 = "api_key=" + apiKey + "&field3=" + String(c);          
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      int httpResponseCode2 = http.POST(httpRequestData2);
      Serial.print("hi");

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
void room_temp(){
  
}
void heart_sensor(){
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
   
}
////// ------ thingsspeak---------/////


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
      String httpRequestData = "api_key=" + apiKey + "&field1=" + String(beatsPerMinute)+"&field3=" + String(700);           
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
Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

   if(beatsPerMinute>90)
  {
    //digitalWrite(LED, HIGH); // turn the LED on
    analogWrite(LED, brightness);

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness == 0 || brightness == 255) {
    fadeAmount = -fadeAmount ;
  }
  // wait for 30 milliseconds to see the dimming effect
  //delay(30);
  }
  else{
    analogWrite(LED, 0);
  }

  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();
}
void gyro(){
  double Ax, Ay, Az;
  double xvalue,yvalue,zvalue;
  double xvalue1,yvalue1,zvalue1;
  double xvalue2,yvalue2,zvalue2;
  double dx,dy,dz,T;
  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
    //divide each with their sensitivity scale factor
  Ax = (double)AccelX/AccelScaleFactor;
  Ay = (double)AccelY/AccelScaleFactor;
  Az = (double)AccelZ/AccelScaleFactor;
  T = (double)Temperature/340+36.53;
  xvalue = Ax -1.03;
  yvalue = Ay +0.06;
  zvalue = Az -0.07;
if (dcount%2 ==0)
{
  xvalue1 = xvalue;
  yvalue1 = yvalue;
  zvalue1 = zvalue;
  dcount++;
}
else
{
 xvalue2 = xvalue;
 yvalue2 = yvalue;
 zvalue2 = zvalue;
 dcount++;
 dx = xvalue2-xvalue1;
 dy = yvalue2-yvalue1;
 dz = zvalue2-zvalue1;
delay(1100);

 if(((dx < MinValue) || (dx > MaxValue)  || (dy < MinValue) || (dy > MaxValue)  || (dz < MinValue) || (dz > MaxValue)) && (timer+millis()>12000))
 {count++;
  if(count>10)
  {if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      //String httpRequestData = "api_key=" + apiKey + "&field5 =" + String(count);
      String httpRequestData2 = "api_key=" + apiKey + "&field6 =" + String(T); 
     // String httpRequestData3 = "api_key=" + apiKey + "&field3 =" + String(c); 
               
      // Send HTTP POST request
      int httpResponseCode2 = http.POST(httpRequestData2);
     

Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode2);
        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }

//////////// ----- thingsspeak -----/////////
    if(mailcount<2)
  {
  delay(500);
  wifi_123();
  gps_123(); 
  mail(); 
  for(int i=1;i<5;i++){
    if(i%2==0)
    analogWrite(LED2, 255);
     if(i%2!=0)
     analogWrite(LED2, 0);
     
  
  }
  //byte ret = sendEmail();
  }
    mailcount++; 
 }
}
}
}
void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}
void MPU6050_Init(){
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}
void gps_123()
{
while ((Serial.available() > 0)&&(gpscount<2))
{
    gpscount++;
    if (gps.encode(Serial.read()))
    {
      if (gps.location.isValid())
      {
        gpscount++;
        latitude = gps.location.lat();
        lat_str = String(latitude , 6);
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
      }

      if (gps.date.isValid())
      {
        date_str = "";
        date = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();
        if (date < 10)
          date_str = '0';
        date_str += String(date);
        date_str += " / ";
        if (month < 10)
          date_str += '0';
        date_str += String(month);
        date_str += " / ";
        if (year < 10)
          date_str += '0';
        date_str += String(year);
      }
      if (gps.time.isValid())
      {
        time_str = "";
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();
        minute = (minute + 30);
        if (minute > 59)
        {
          minute = minute - 60;
          hour = hour + 1;
        }
        hour = (hour + 5) ;
        if (hour > 23)
          hour = hour - 24;
        if (hour >= 12)
          pm = 1;
        else
          pm = 0;
        hour = hour % 12;
        if (hour < 10)
          time_str = '0';
        time_str += String(hour);
        time_str += " : ";
        if (minute < 10)
          time_str += '0';
        time_str += String(minute);
        time_str += " : ";
        if (second < 10)
          time_str += '0';
        time_str += String(second);
        if (pm == 1)
          time_str += " PM ";
        else
          time_str += " AM ";
         Serial.print("Date= ");
         Serial.println(date_str);
         Serial.print("Time= ");
         Serial.println(time_str);
         Serial.print("Lat= ");
         Serial.println(lat_str);
         Serial.print("Long= ");
         Serial.println(lng_str); 
       //  delay(500);
                 
      }
    }
}
}
void mail()
{
   if (client.connected())
  {
    client.stop();
  }

  client.flush();
  if (client.connect(HOSTIFTTT,80)) {
    Serial.println("Connected");
    // build the HTTP request
    String toSend = "GET /trigger/";
    toSend += EVENTO;
    toSend += "/with/key/";
    toSend += IFTTTKEY;
    toSend += "?value1=";
    toSend += lat_str;
    toSend += "&value2=";
    toSend += lng_str;
    toSend += " HTTP/1.1\r\n";
    toSend += "Host: ";
    toSend += HOSTIFTTT;
    toSend += "\r\n";
    toSend += "Connection: close\r\n\r\n";
    client.print(toSend);
    //delay(250);
  }
  client.flush();
  client.stop();  
  }
  void wifi_123()
{WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("*");
  }
  Serial.println("");
  Serial.println("WiFi Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
void alcohol(){
  bool digital = digitalRead(sensorDigital);
  int analog = analogRead(sensorAnalog);
 
  Serial.print("Analog value : ");
  Serial.print(analog);
  Serial.print("t");
  Serial.print("Digital value :");
  Serial.println(digital);
  if(analog>900){
    digitalWrite(D3,HIGH);
  }
  else{
    digitalWrite(D3,LOW);
  }
  c=analog;
  ////// ------ thingsspeak---------/////


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
      String httpRequestData = "api_key=" + apiKey + "&field3=" + String(analog);           
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
