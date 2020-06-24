#include <SoftwareSerial.h>
#define DEBUG true
 
SoftwareSerial esp8266(2,3); 
void setup() {
  Serial.begin(9600);
  esp8266.begin(9600);
  
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  
  pinMode(11, OUTPUT);
  digitalWrite(11, LOW);
  
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
   
  //sendData("AT+RST\r\n",2000,DEBUG); // esp-01 리셋
  //sendData("AT+CIOBAUD?\r\n",2000,DEBUG); // 보드레이트 체크
  //sendData("AT+CWMODE=3\r\n",1000,DEBUG); // 모드 설정
  //sendData("AT+CWLAP\r\n",3000,DEBUG); // 연결가능한 wifi목록 
  sendData("AT+CWJAP=\"khu\",\"87654321\"\r\n",5000,DEBUG); // kyu-wifi 연결
  sendData("AT+CIFSR\r\n",1000,DEBUG); // ip주소 출력
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
}
 
void loop() {
  if(esp8266.available()) { 
    if(esp8266.find("+IPD,")) {
      delay(1000); 
      int connectionId = esp8266.read()-48; // subtract 48 because the read() function returns 
                                            // the ASCII decimal value starts at 48
      esp8266.find("pin=");
      int pinNumber = (esp8266.read()-48)*10; 
      pinNumber += (esp8266.read()-48); 
      digitalWrite(pinNumber, !digitalRead(pinNumber)); // toggle pin    
      
     if(pinNumber==10) {                     // 정지
        digitalWrite(pinNumber+1, 1);  
        digitalWrite(pinNumber+2, 1);  
     } else if(pinNumber==11) {             // 역회전
        digitalWrite(pinNumber-1, 1);    
        digitalWrite(pinNumber+1, 1);    
     } if(pinNumber==12) {                  // 정회전
        digitalWrite(pinNumber-1, 1);   
        digitalWrite(pinNumber-2, 1);    
     }

      // make close command
      String closeCommand = "AT+CIPCLOSE="; 
      closeCommand+=connectionId; // append connection id
      closeCommand+="\r\n";
      sendData(closeCommand,1000,DEBUG); // close connection
    }
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
