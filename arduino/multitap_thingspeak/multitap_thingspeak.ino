#include <SoftwareSerial.h>
#include <stdlib.h>
#define DEBUG true

String apiKey = "O3NLRW52H9W83CQP";

float getVPP0();
float getVPP1();
float getVPP2();
const int sensorIn0 = A0;
const int sensorIn1 = A1;
const int sensorIn2 = A2;
int mVperAmp = 100; // use 185 for 5A Module and 66 for 30A Module

double Voltage0 = 0;
double VRMS0 = 0;
double AmpsRMS0 = 0; 
double Voltage1 = 0;
double VRMS1 = 0;
double AmpsRMS1 = 0;
double Voltage2 = 0;
double VRMS2 = 0;
double AmpsRMS2 = 0;
double sum=0;
 
SoftwareSerial esp8266(2,3); // TX/RX 설정, esp8266 객체생성
 
void setup() {

  Serial.begin(9600); 
  esp8266.begin(9600);
 
 pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
   /*
  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  sendData("AT+CIOBAUD?\r\n",2000,DEBUG); // check baudrate (redundant)
  sendData("AT+CWMODE=3\r\n",1000,DEBUG); // configure as access point (working mode: AP+STA)
  sendData("AT+CWLAP\r\n",3000,DEBUG); // list available access points
  */
  sendData("AT+CWJAP=\"khu\",\"87654321\"\r\n",5000,DEBUG); // join the access point
  
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80할 공유기 설정
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
}
 
void loop() {
if(esp8266.available()) { // check if the esp is sending a message
    if(esp8266.find("+IPD,")) {
      delay(1000); // wait for the serial buffer to fill up (read all the serial data)
      // get the connection id so that we can then disconnect
      int connectionId = esp8266.read()-48; // subtract 48 because the read() function returns 
                                           // the ASCII decimal value and 0 (the first decimal number) starts at 48
      esp8266.find("pin="); // advance cursor to "pin="
      int pinNumber = (esp8266.read()-48)*10; // get first number i.e. if the pin 13 then the 1st number is 1, then multiply to get 10
      pinNumber += (esp8266.read()-48); // get second number, i.e. if the pin number is 13 then the 2nd number is 3, then add to the first number
      digitalWrite(pinNumber, !digitalRead(pinNumber)); // toggle pin    

      // make close command
      String closeCommand = "AT+CIPCLOSE="; 
      closeCommand+=connectionId; // append connection id
      closeCommand+="\r\n";
      sendData(closeCommand,1000,DEBUG); // close connection
    }
  }
 Voltage0 = getVPP0();
 VRMS0 = (Voltage0/2.0) *0.707; 
 AmpsRMS0 = (VRMS0 * 1000)/mVperAmp;
 
 Voltage1 = getVPP1();
 VRMS1 = (Voltage1/2.0) *0.707; 
 AmpsRMS1 = (VRMS1 * 1000)/mVperAmp;
 
 Voltage2 = getVPP2();
 VRMS2 = (Voltage2/2.0) *0.707; 
 AmpsRMS2 = (VRMS2 * 1000)/mVperAmp;

 sum = AmpsRMS0+AmpsRMS1+AmpsRMS2;

 Serial.print(" Amps RMS0 : \t  ");
 Serial.print(AmpsRMS0);
 Serial.print(" Amps RMS1 : \t  ");
 Serial.print(AmpsRMS1);
 Serial.print(" Amps RMS2 : \t");
 Serial.println(AmpsRMS2);
  
  // TCP 연결
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com 접속 IP
  cmd += "\",80";           // api.thingspeak.com 접속 포트, 80
  esp8266.println(cmd);
   
  if(esp8266.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }
 
  
  // GET 방식으로 보내기 위한 String, Data 설정
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(AmpsRMS0);
  getStr +="&field2=";
  getStr += String(AmpsRMS1);
  getStr +="&field3=";
  getStr += String(AmpsRMS2);
  getStr +="&field4=";
  getStr += String(sum);
  getStr += "\r\n\r\n";
  
 
  // Send Data
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  esp8266.println(cmd);
 
  if(esp8266.find(">")){
    esp8266.print(getStr);
  }
   
  // Thingspeak 최소 업로드 간격 15초를 맞추기 위한 delay
  delay(2000);  
}

 
/*ESP8266의 정보를 알아내고 설정하기 위한 함수 선언*/
String sendData(String command, const int timeout, boolean debug){
  String response = "";
  esp8266.print(command); //command를 ESP8266에 보냄
  long int time=millis();
  
  while((time+timeout)>millis()){
    while(esp8266.available()){
      /*esp가 가진 데이터를 시리얼 모니터에 출력하기 위함*/
      char c=esp8266.read(); //다음 문자를 읽어옴
      response+=c;
    }
  }
  if(debug){
    Serial.print(response);
  }
 
  return response;
}

float getVPP0()
{
  float result0;
  
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn0);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the maximum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result0 = ((maxValue - minValue) * 5.0)/1024.0;
      
   return result0;
 }

 float getVPP1()
{
  float result1;
  
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn1);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the maximum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result1 = ((maxValue - minValue) * 5.0)/1024.0;
      
   return result1;
 }

 float getVPP2()
{
  float result2;
  
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn1);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the maximum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result2 = ((maxValue - minValue) * 5.0)/1024.0;
      
   return result2;
 }
