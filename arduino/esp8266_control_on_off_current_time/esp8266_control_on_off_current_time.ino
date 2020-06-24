#include <SoftwareSerial.h>
#define DEBUG true

float getVPP0();
const int sensorIn0 = A0;
const int sensorIn1 = A1;
const int sensorIn2 = A2; // 12
int mVperAmp = 100; // use 185 for 5A Module and 66 for 30A Module

double Voltage0 = 0;
double VRMS0 = 0;
double AmpsRMS0 = 0; 
double AmpsRMS0_mA = 0;

 long time;
 
SoftwareSerial esp8266(2,3); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                                        // This means that you need to connect the TX line from the esp to the Arduino's pin 2
                                        // and the RX line from the esp to the Arduino's pin 3
 
void setup() {
  Serial.begin(9600);
  esp8266.begin(9600); // your esp's baud rate might be different
  
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
 // sendData("AT+CWLAP\r\n",3000,DEBUG); // list available access points
 */
  sendData("AT+CWJAP=\"khu\",\"87654321\"\r\n",5000,DEBUG); // join the access point
  
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
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
 AmpsRMS0_mA = AmpsRMS0*1000;

 Serial.print("Amps RMS0 : ");
 Serial.println(AmpsRMS0_mA);
 delay(3000);

    if((120<=AmpsRMS0_mA) && (AmpsRMS0_mA<=140)){

    if((millis()-time)>=2000){ digitalWrite (10 ,LOW); }

  }

  else{

    time=millis() ;

  }
}
 
String sendData(String command, const int timeout, boolean debug) {
    String response = "";
    esp8266.print(command); // send the read character to the esp8266
    long int time = millis();
    
    while( (time+timeout) > millis()) {
      while(esp8266.available()) {
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }
    }
    
    if(debug) {
      Serial.print(response);
    }
    return response;
}

float getVPP0()
{
  float result0;
  
  int readValue0;             //value read from the sensor
  int maxValue0 = 0;          // store max value here
  int minValue0 = 1024;          // store min value here
  
   uint32_t start_time0 = millis();
   while((millis()-start_time0) < 1000) //sample for 1 Sec
   {
       readValue0 = analogRead(sensorIn0);
       // see if you have a new maxValue
       if (readValue0 > maxValue0) 
       {
           /*record the maximum sensor value*/
           maxValue0= readValue0;
       }
       if (readValue0 < minValue0) 
       {
           /*record the maximum sensor value*/
           minValue0 = readValue0;
       }
   }
   
   // Subtract min from max
   result0 = ((maxValue0 - minValue0) * 5.0)/1024.0;
      
   return result0;
 }

