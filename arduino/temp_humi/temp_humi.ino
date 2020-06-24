#include <SoftwareSerial.h>  //including the software serial UART library which will make the digital pins as TX and RX
#include "DHT.h"             //including the DHT22 library
#define DHTPIN 8             //Declaring pin 8 of arduino to communicate with DHT22
#define DHTTYPE DHT22        //Defining type of DHT sensor we are using (DHT22 or DHT11)
#define DEBUG true
DHT dht(DHTPIN, DHTTYPE);    //Declaring a variable named dht
SoftwareSerial esp8266(2,3); //Connect the TX pin of ESP8266 to pin 2 of Arduino and RX pin of ESP8266 to pin 3 of Arduino.

void setup()
{
  Serial.begin(9600);
  esp8266.begin(9600); // Set the baud rate of serial communication
  dht.begin();         //This will initiate receiving data from DHT22

  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  sendData("AT+CIOBAUD?\r\n",2000,DEBUG); // check baudrate (redundant)
  sendData("AT+CWMODE=3\r\n",1000,DEBUG); // configure as access point (working mode: AP+STA)
  sendData("AT+CWLAP\r\n",3000,DEBUG); // list available access points
  sendData("AT+CWJAP=\"khu\",\"87654321\"\r\n",5000,DEBUG); // join the access point
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
}

void loop()
{
  float hum = dht.readHumidity();     //Reading humidity and storing in hum
  float temp = dht.readTemperature(); //Reading temperature in celsius and storing in temp
  // Check if any reads failed and exit early (to try again)
  float f = dht.readTemperature(true);
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  float hi = dht.computeHeatIndex(f, hum); // Computing heat index in Fahrenheit
  float hiDegC = dht.convertFtoC(hi);      // Converting heat index to degree centigrade

  if(esp8266.available())     // If any data is available to read (only when we request data through browser)
  {
    if(esp8266.find("+IPD,")) // Validating the request by finding "+IPD" in the received data
    {
      delay(1000);
      int connectionId = esp8266.read()-48;                // Subtracting 48 from the character to get the connection id.
      String webpage = "<h1>        Current Temperature&Humidiy</h1>"; // Creating a string named webpage and storing the data in it. 
 
      webpage += "<h3>        Temperature: ";
      webpage += temp;                                     //This will show the temperature on the webpage.
      webpage += "*C";
      webpage += "</h3>";

      webpage += "<h3>        Humidity: ";
      webpage += hum;
      webpage += "%";
      webpage += "</h3>";

      String cipSend = "AT+CIPSEND=";
      cipSend += connectionId;
      cipSend += ",";
      cipSend +=webpage.length();
      cipSend +="\r\n";

      sendData(cipSend,1000,DEBUG);
      sendData(webpage,1000,DEBUG); // Sending temperature and humidity information to browser

      //The following three commands will close the connection
      String closeCommand = "AT+CIPCLOSE=";
      closeCommand+=connectionId;   // append connection id
      closeCommand+="\r\n";

      sendData(closeCommand,3000,DEBUG); // Sending the close command to the sendData function to execute
    }
  }
}

//This function will send the data to the webpage
String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  esp8266.print(command);      // Sending command to ESP8266 module
  long int time = millis();    // Waiting for sometime 
  while( (time+timeout) > millis())
  {
    while(esp8266.available()) // Checking whether the ESP8266 has received the data or not
    {
      char c = esp8266.read(); // Reading response from ESP8266
      response+=c;
    }
  }
  if(debug)
  {
    Serial.print(response);
  }
  return response;
}
