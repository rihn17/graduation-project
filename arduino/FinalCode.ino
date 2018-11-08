#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define NUM_LEDS 60
#define BRIGHTNESS 50
#define TIMEOUT 10000 // mS

String json;
bool res;
int tryNum;
int num=0;
int resultAge = 0;
int resultId = 0;
String resultGender;
int resultSafe;
int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};
int gamma[] = {
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

SoftwareSerial swSerial(2, 3); // RX, TX
LiquidCrystal_I2C lcd(0x27,16,2);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);

//LCD, LED 초기화
//통신 시작, 와이파이 연결
void setup(){
  LCD_Off();
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show();
  Light_Off();
  Serial.begin(9600);
  swSerial.begin(9600);
  connectWiFi();
}

//api 값 수신, 값에 따라 if-else문 수행
void loop() {
  acquireInfo();
  if (!resultSafe)  //false = 0 true = 1
  {
    LCD_On();
    switch(num){
      case 0: colorWipe(strip.Color(255, 0, 0), 30);
              break; 
      case 1: colorWipe(strip.Color(0, 255, 0), 30);
              break;
      case 2: colorWipe(strip.Color(0, 0, 255), 30);
              break;
      case 3: colorWipe(strip.Color(0, 0, 0, 255), 30);
              break;
      case 4: pulseWhite(0); 
              rainbow(10);
              break;
      case 5: rainbowCycle(3);
              break;
      case 6: whiteOverRainbow(5,75,5);
              break;
    }
    alarm();
    if(num < 6)  num++;
    else num = 0;
  }
  else
  {
    LCD_Off();
    Light_Off();
  }
}

//소리
void alarm(){
  for (int i = 0; i < 8; i++) {
    tone(8, melody[i], 250);
    delay(400);
    noTone(8);
  }
}

boolean SendCommand(String cmd, String ack){
  swSerial.println(cmd); // 모듈에 AT Command 전송
  if (!echoFind(ack, TIMEOUT))
    return true; // ack blank
    else return false; //  ack found
}
 
boolean echoFind(String keyword, int timeout){
  json = "";
  byte current_char = 0;
  byte keyword_length = keyword.length();
  long deadline = millis() + timeout;
  bool match = false;
  bool jsonflag = false;
  while(millis() < deadline){
    if (swSerial.available()){
      char ch = swSerial.read();
      Serial.write(ch);
      if(ch == '{') jsonflag = true;
      if(jsonflag == true) json += ch;
      if (ch == keyword[current_char]) {
        if (++current_char == keyword_length) return true;
        match = true;
      } else {
        match = false;
        current_char = 0;
      }
    }
  }
  return false; // Timed out
}

int connectWiFi(void) {  
  tryNum = 1;
  while(tryNum<5) {
    Serial.print(F("\nConnecting to AP... (no. "));
    Serial.print(tryNum);
    Serial.println(F(")"));
    res = SendCommand(F("AT+CWJAP=\"hello2\",\"01065873639\""),F("OK"));
    if(res == false) break;
    else tryNum++;
  }
  if(tryNum == 5) return -1;
  Serial.println(F("\nConnect Success."));
}

int acquireInfo(void) {
  String cmd = F("AT+CIPSTART=\"TCP\",\"http://babymonitor.pythonanywhere.com\",80");
  swSerial.println(cmd);
  delay(2000);
  if(swSerial.find("ERROR")){
    return -1;
  }
  cmd = F("GET /api/get/all HTTP/1.1");
  cmd += "\r\n";
  cmd += F("Host: babymonitor.pythonanywhere.com");
  cmd += "\r\n";
  swSerial.print(F("AT+CIPSEND="));
  swSerial.println(cmd.length()+2);
  if(swSerial.find(">")){
    swSerial.print(cmd);
    res = SendCommand("",F("}"));
  }else{
    swSerial.println(F("AT+CIPCLOSE"));
    return -1;
  }
  swSerial.println(F("AT+CIPCLOSE"));
  
  resultAge = ageFind();
  resultId = idFind();
  resultGender = genderFind();
  resultSafe = safeFind();
 
  if(res == false) {
    Serial.print(F("\n(AGE = "));
    Serial.print(resultAge);
    Serial.println(F(")"));
    Serial.print(F("\nID = "));
    Serial.print(resultId);
    Serial.println(F(")"));
    Serial.print(F("\n(GENDER = "));
    Serial.print(resultGender);
    Serial.println(F(")"));
    Serial.print(F("\n(SAFE = "));
    Serial.print(resultSafe);
    Serial.println(F(")"));
  } 
  Serial.println(F("\nClosing connection to host."));
  swSerial.println(F("AT+CIPCLOSE"));
}

int ageFind() {
  int ageIdx = json.indexOf("age\":");
  int ageEndIdx = json.indexOf(",", ageIdx+1);
  String resultAge = json.substring(ageIdx+5, ageEndIdx);
  return (int)resultAge.toInt();
}

int idFind() {
  int idIdx = json.indexOf("camId");
  int idEndIdx = json.indexOf(",", idIdx+1);
  String resultId = json.substring(idIdx+7, idEndIdx);
  return (int)resultId.toInt();
}

String genderFind() {
  int genderIdx = json.indexOf("gender");
  int genderEndIdx = json.indexOf("\"", genderIdx+9);
  String resultGender = json.substring(genderIdx+9, genderEndIdx);
  return resultGender;
}

int safeFind() {
  int safeidx = json.indexOf("isSafe");
  int safeEndidx = json.indexOf("}", safeidx+1);
  String safe = json.substring(safeidx+8, safeEndidx);
  int resultSafe;
  if(safe.equals("true"))
    resultSafe = 1;
   else
    resultSafe = 0;
  return resultSafe;
}

void LCD_On(){
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Warning!!!");
  lcd.setCursor(6,1);
  lcd.print("CAM");
  lcd.print(resultId);
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void pulseWhite(uint8_t wait) {
  for(int j = 0; j < 256 ; j++){
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(0,0,0, gamma[j] ) );
        }
        delay(wait);
        strip.show();
      }

  for(int j = 255; j >= 0 ; j--){
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i, strip.Color(0,0,0, gamma[j] ) );
        }
        delay(wait);
        strip.show();
      }
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

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  
  for(j=0; j<256 * 5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input 0~255
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3,0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3,0);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0,0);
}

uint8_t red(uint32_t c) {
  return (c >> 8);
}
uint8_t green(uint32_t c) {
  return (c >> 16);
}
uint8_t blue(uint32_t c) {
  return (c);
}

void LCD_Off(){
  lcd.init();
  lcd.noBacklight();
}

void Light_Off(){
   for(int i=0;i<NUM_LEDS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    strip.setPixelColor(i, strip.Color(0,0,0)); 
    strip.show(); // This sends the updated pixel color to the hardware.
  }
}

