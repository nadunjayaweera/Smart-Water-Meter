#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <SD.h>
#include<SPI.h>
#include <SoftwareSerial.h>

unsigned long prevTime = millis();
int X;
int Y;
float TIME = 0;
float FREQUENCY = 0;
float WATER = 0;
float TOTAL = 0;
float LS = 0;
int button1State = 1;// flowter sensor
int button2State = 1;// casing safty 
int button3State = 1;// pipe line safty

const int input = 16; // D0 digital pin flowsensor
int valve = 0; // digital pin D3 Relay module
int floatSensor = 5; // D1 digital pin flowter sensor
int pipeline = 4; // D2 digital pin pipeline cutt
int casing = 2; // D4 digital pin cassing open 

//Wi-Fi name.
const char* ssid     = "SLT-ADSL-86938";
//Wi-Fi Pass word
const char* password = "caj2753nmj";
// Divice apikey for identify device. each device has deverent api keys.
String apiKeyValue = "12345678";
// Link using get and post data to server.
const char* host = "192.168.1.2:8080/api/v1/data";



void setup() {
  // Initialize "debug" serial port
  Serial.begin(4800);
  // Defining input and output pins
  pinMode(input,INPUT);
  pinMode(floatSensor, INPUT_PULLUP);
  pinMode(pipeline, INPUT_PULLUP);
  pinMode(casing, INPUT_PULLUP); 
  pinMode(valve, OUTPUT);
  digitalWrite(valve, LOW); //Turn OFF valve
  digitalWrite(LED_BUILTIN, HIGH);

  //Connectiong to the wifi.
   WiFi.begin(ssid, password);
  Serial.println("Connecting");

  // trying to connect wifi.
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  
  }
   Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop(){
  
  unsigned long currentTime = millis();
  String dataString = "";
  String dataString2 = "";
  String dataString3 = "";
  String dataString5 = "";
  String dataString6 = "";

  // assigning values to variables.
  button1State = digitalRead(floatSensor);
  button2State = digitalRead(pipeline);
  button3State = digitalRead(casing);

//Measure water flow.
X = pulseIn(input, HIGH);
Y = pulseIn(input, LOW);
TIME = X + Y;
FREQUENCY = 1000000/TIME;
WATER = FREQUENCY/7.5;
LS = WATER/60;
if(FREQUENCY >= 0)
{
if(isinf(FREQUENCY))
{

}
else
{
TOTAL = TOTAL + LS;
}
}

//Sending data to server every 8 second without hold the void loop function
if(currentTime - prevTime >8000){
  Serial.print("TOTAL:");
Serial.print( TOTAL);
Serial.println(" L");
Serial.println("---");
prevTime = currentTime;

    dataString = String(TOTAL);
    dataString2 = String(button1State);
    dataString3 = String(button2State); 
    dataString5 = String(button3State);
    
  // Create json doc for send to server.
    DynamicJsonDocument doc(1024);

    doc["apiKey"] = apiKeyValue;
    doc["value1"] = dataString; //water count
    doc["value2"] = dataString2; // Flowter sensor
    doc["value3"] = dataString3; // Pipe Line
    doc["value4"] = dataString5; // casing 


////////////////////////////////data sending part//////////////////////////////////////////////////////////////////////

  if(WiFi.status() == WL_CONNECTED)
  {
    
    WiFiClient client;
    HTTPClient http;  // Declare object of class HTTPClient.
 
  
    
    String getData;
    http.begin(client, host);
    // Specify content-type header
    http.addHeader("Content-Type", "application/json");
    String httpRequestData = "";
    serializeJson(doc, httpRequestData);
    Serial.print("httpRequestData: ");
    // reset total value in flow sensor.
    TOTAL = 0;
    // post data to server.
    Serial.println(httpRequestData);
    int httpResponseCode = http.POST(httpRequestData);

    // Getting http response code.
     if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    String payloadGetpart = http.getString();
    Serial.print("Incomming data : ");
    Serial.println(payloadGetpart);
    // Free resources
    // Check server response and aire detection for open and close valve.


    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
//////////////Get_Request////////////////////////////////////////
WiFiClient client;
HTTPClient http;
http.begin(client, "192.168.1.2:8080/api/v1/data");

    String payloadincomming = http.getString();
    int httpResponseCodeget = http.GET();
    if(httpResponseCodeget>0){
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCodeget);
      payloadincomming = http.getString();
      Serial.print("Incomming data: ");
      Serial.println(payloadincomming);
    }
http.end();
Serial.println();
if (payloadincomming == "0"){
  //Open valve
  digitalWrite(valve, HIGH); 
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Valve OFF");
}
if ( payloadincomming =="1"){
  //Close valve
  digitalWrite(valve, LOW); 
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Valve ON");
  
}

Serial.println("---------------End OF Message---------------");
Serial.println("");
}
}
