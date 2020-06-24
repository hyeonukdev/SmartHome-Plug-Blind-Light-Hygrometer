#include <MsTimer2.h>
#include <ESP8266.h>
#include <ESP8266Client.h>
#include <SoftwareSerial.h>
#define DEBUG true

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

boolean Plug0 = false;
boolean Plug1 = false;
boolean Plug2 = false;

SoftwareSerial esp8266(2, 3); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
// This means that you need to connect the TX line from the esp to the Arduino's pin 2
// and the RX line from the esp to the Arduino's pin 3

void setup() {
  Serial.begin(9600);
  esp8266.begin(9600); // your esp's baud rate might be different

  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);

  pinMode(11, OUTPUT);
  digitalWrite(11, LOW);

  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
 /*
    sendData("AT+RST\r\n",2000,DEBUG); // reset module
    sendData("AT+CIOBAUD?\r\n",2000,DEBUG); // check baudrate (redundant)
    sendData("AT+CWMODE=3\r\n",1000,DEBUG); // configure as access point (working mode: AP+STA)
    sendData("AT+CWLAP\r\n",3000,DEBUG); // list available access points
    sendData("AT+CWJAP=\"kyu-wifi\",\"\"\r\n",5000,DEBUG); // join the access point
  */
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG); // configure for multiple connections
  sendData("AT+CIFSR\r\n", 1000, DEBUG); // get ip address
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG); // turn on server on port 80
}

void loop() {
  if (esp8266.available()) { // check if the esp is sending a message
    if (esp8266.find("+IPD,")) {
      delay(1000); // wait for the serial buffer to fill up (read all the serial data)
      // get the connection id so that we can then disconnect
      int connectionId = esp8266.read() - 48; // subtract 48 because the read() function returns
      // the ASCII decimal value and 0 (the first decimal number) starts at 48
      esp8266.find("pin="); // advance cursor to "pin="
      int pinNumber = (esp8266.read() - 48) * 10; // get first number i.e. if the pin 13 then the 1st number is 1, then multiply to get 10
      pinNumber += (esp8266.read() - 48); // get second number, i.e. if the pin number is 13 then the 2nd number is 3, then add to the first number
      digitalWrite(pinNumber, !digitalRead(pinNumber)); // toggle pin

      if (pinNumber == 12)
        Plug0 = true;
      if (pinNumber == 11)
        Plug1 = true;
      if (pinNumber == 10)
        Plug2 = true;

      String webpage = "<h1>plug1</h1>";
      String cipsend = "AT+CIPSEND=";
      cipsend += connectionId;
      cipsend += ",";
      cipsend += webpage.length();
      cipsend += "/r/n";

      sendData(cipsend, 1000, DEBUG);
      sendData(webpage, 1000, DEBUG);

      // make close command
      String closeCommand = "AT+CIPCLOSE=";
      closeCommand += connectionId; // append connection id
      closeCommand += "\r\n";
      sendData(closeCommand, 1000, DEBUG); // close connection

    }
  }

  Voltage0 = getVPP0();
  VRMS0 = (Voltage0 / 2.0) * 0.707;
  AmpsRMS0 = (VRMS0 * 1000) / mVperAmp;
  Voltage1 = getVPP1();
  VRMS1 = (Voltage1 / 2.0) * 0.707;
  AmpsRMS1 = (VRMS1 * 1000) / mVperAmp;
  Voltage2 = getVPP2();
  VRMS2 = (Voltage2 / 2.0) * 0.707;
  AmpsRMS2 = (VRMS2 * 1000) / mVperAmp;

  Serial.print(" Amps RMS0 : ");
  Serial.print(AmpsRMS0);
  Serial.print('\t');
  Serial.print(" Amps RMS1 : ");
  Serial.print(AmpsRMS1);
  Serial.print('\t');
  Serial.print(" Amps RMS2 : ");
  Serial.println(AmpsRMS2);

  if (AmpsRMS0 <= 0.14)
  {
    Serial.print("Plug0 Status : ");
    Serial.print("OFF \t");
  } else {
    Serial.print("Plug0 Status : ");
    Serial.print("ON \t");
  }

  if (AmpsRMS1 <= 0.14)
  {
    Serial.print("Plug1 Status : ");
    Serial.print("OFF \t");
  } else {
    Serial.print("Plug1 Status : ");
    Serial.print("ON \t");
  }

  if (AmpsRMS2 <= 0.14) {
    Serial.print("Plug1 Status : ");
    Serial.println("OFF \t");
  } else {
    Serial.print("Plug1 Status : ");
    Serial.println("ON");
  }






}


String sendData(String command, const int timeout, boolean debug) {
  String response = "";
  esp8266.print(command); // send the read character to the esp8266
  long int time = millis();

  while ( (time + timeout) > millis()) {
    while (esp8266.available()) {
      // The esp has data so display its output to the serial window
      char c = esp8266.read(); // read the next character.
      response += c;
    }
  }

  if (debug) {
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
  while ((millis() - start_time) < 1000) //sample for 1 Sec
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
  result0 = ((maxValue - minValue) * 5.0) / 1024.0;

  return result0;
}

float getVPP1()
{
  float result1;

  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
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
  result1 = ((maxValue - minValue) * 5.0) / 1024.0;

  return result1;
}

float getVPP2()
{
  float result2;

  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) //sample for 1 Sec
  {
    readValue = analogRead(sensorIn2);
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
  result2 = ((maxValue - minValue) * 5.0) / 1024.0;

  return result2;
}



