#include <SoftwareSerial.h>
#define DEBUG true
 
SoftwareSerial esp8266(2,3); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                                        // This means that you need to connect the TX line from the esp to the Arduino's pin 2
                                        // and the RX line from the esp to the Arduino's pin 3
  #include <Adafruit_NeoPixel.h>
 #ifdef __AVR__
  #include <avr/power.h>
  #endif
  #define PIN 6
  
#define NUM_LEDS 31 //led갯
#define BRIGHTNESS b //led 밝(~255)
int b=BRIGHTNESS;
 Adafruit_NeoPixel strip = Adafruit_NeoPixel(35, PIN, NEO_GRBW + NEO_KHZ800);
byte neopix_gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
  
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
void setup() {
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
      #endif
      b=100;
        strip.setBrightness(b);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
  esp8266.begin(9600); // your esp's baud rate might be different
  

   
  sendData("AT+RST\r\n",2000,DEBUG); // reset module
  sendData("AT+CIOBAUD?\r\n",2000,DEBUG); // check baudrate (redundant)
  sendData("AT+CWMODE=3\r\n",1000,DEBUG); // configure as access point (working mode: AP+STA)
  sendData("AT+CWLAP\r\n",3000,DEBUG); // list available access points
  sendData("AT+CWJAP=\"khu\",\"87654321\"\r\n",5000,DEBUG); // join the access point
  sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
}
uint8_t red(uint32_t c) {
  return (c >> 16);
}
uint8_t green(uint32_t c) {
  return (c >> 8);
}
uint8_t blue(uint32_t c) {
  return (c);
}
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
 void rainbowFade2White(uint8_t wait, int rainbowLoops, int whiteLoops) {
  float fadeMax = 100.0;
  int fadeVal = 0;
  uint32_t wheelVal;
  int redVal, greenVal, blueVal;

  for(int k = 0 ; k < rainbowLoops ; k ++){
    
    for(int j=0; j<256; j++) { // 5 cycles of all colors on wheel

      for(int i=0; i< strip.numPixels(); i++) {

        wheelVal = Wheel(((i * 256 / strip.numPixels()) + j) & 255);

        redVal = red(wheelVal) * float(fadeVal/fadeMax);
        greenVal = green(wheelVal) * float(fadeVal/fadeMax);
        blueVal = blue(wheelVal) * float(fadeVal/fadeMax);

        strip.setPixelColor( i, strip.Color( redVal, greenVal, blueVal ) );

      }

      //First loop, fade in!
      if(k == 0 && fadeVal < fadeMax-1) {
          fadeVal++;
      }

      //Last loop, fade out!
      else if(k == rainbowLoops - 1 && j > 255 - fadeMax ){
          fadeVal--;
      }

        strip.show();
        delay(wait);
    }
  
  }



  delay(500);


  for(int k = 0 ; k < whiteLoops ; k ++){

    for(int j = 0; j < 256 ; j++){

        for(uint16_t i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(0,0,0, neopix_gamma[j] ) );
          }
          strip.show();
        }

        delay(2000);
    for(int j = 255; j >= 0 ; j--){

        for(uint16_t i=0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(0,0,0, neopix_gamma[j] ) );
          }
        }
  }

  delay(500);


}
 void whiteOverRainbow(uint8_t wait, uint8_t whiteSpeed, uint8_t whiteLength ) {
  
  if(whiteLength >= strip.numPixels()) whiteLength = strip.numPixels() - 1;

  int head = whiteLength - 1;
  int tail = 0;

  int loops = 3;
  int loopNum = 0;

  static unsigned long lastTime = 0;


  while(true){
    for(int j=0; j<256; j++) {
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        if((i >= tail && i <= head) || (tail > head && i >= tail) || (tail > head && i <= head) ){
          strip.setPixelColor(i, strip.Color(0,0,0, 255 ) );
        }
        else{
          strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        
      }

      if(millis() - lastTime > whiteSpeed) {
        head++;
        tail++;
        if(head == strip.numPixels()){
          loopNum++;
        }
        lastTime = millis();
      }

      if(loopNum == loops) return;
    
      head%=strip.numPixels();
      tail%=strip.numPixels();
        strip.show();
        delay(wait);
    }
  }
  
}
void loop() {
  if(esp8266.available()) { // check if the esp is sending a message
    if(esp8266.find("+IPD,")) {
      delay(1000);
            int connectionId = esp8266.read()-48;  
                                           
      esp8266.find("pin="); // advance cursor to "pin="
      int pinNumber = (esp8266.read()-48)*10;
      pinNumber += (esp8266.read()-48); 
      digitalWrite(pinNumber, !digitalRead(pinNumber)); // toggle pin    

      strip.begin(); // 네오픽셀 제어를 시작
           if(pinNumber==10){
for(int a=0;a<35;a++)
strip.setPixelColor(a,0,0,0,0);
}
     if(pinNumber==11){
    colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  colorWipe(strip.Color(0, 0, 0, 255), 50); // White
}

if(pinNumber==12){
for(int a=0;a<31;a++)
strip.setPixelColor(a,255,0,0,0);
}
if(pinNumber==13){
for(int a=0;a<31;a++)
strip.setPixelColor(a,0,255,0,0);
}
if(pinNumber==14){
for(int a=0;a<31;a++)
strip.setPixelColor(a,0,0,255,0);
}
if(pinNumber==15){
  for(int a=0;a<31;a++){
strip.setPixelColor(a,0,0,0,255);
strip.setPixelColor(a,255,255,0,0);
  }

}
if(pinNumber==16){//rainbowFade2White
rainbowFade2White(5,5,0);
for(int a=0;a<31;a++)
strip.setPixelColor(a,0,0,0,255);
}

if(pinNumber==17){
  for(int a=0;a<31;a++)
  strip.setPixelColor(a,0,0,0,255);

}
if(pinNumber==18){//밝30
strip.setBrightness(60);

}
if(pinNumber==19){
   strip.setBrightness(120);
}
if(pinNumber==20){
 strip.setBrightness(170);
}
strip.show(); // 네오픽셀을 초기화 해줍니다.

      // make close command
      String closeCommand = "AT+CIPCLOSE="; 
      closeCommand+=connectionId; // append connection id
      closeCommand+="\r\n";
      sendData(closeCommand,1000,DEBUG); // close connection
    }
  }
}
 
