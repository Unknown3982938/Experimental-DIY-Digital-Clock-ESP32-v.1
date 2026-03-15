#include <WiFi.h>
#include <TM1637Display.h>
#include <time.h>

//First display(Time/year, Main) 
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

//Second Display(Date, Secondary) 
#define DATE_CLK 16
#define DATE_DIO 17
TM1637Display dateDisplay(DATE_CLK, DATE_DIO);

//Wifi Configs
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";
const char* ntpServer = "time.google.com";

unsigned long lastNtpSync = 0;
const unsigned long NTP_INTERVAL = 600000;

//Buttons
//DISCLAIMER: Some might not work still/yet
#define BTN_MODE   25
#define BTN_ADJUST 26
#define BTN_SET    27
#define BTN_LEFT   14
#define BTN_RIGHT  12
#define BTN_SOURCE 13

//Others
//DISCLAIMER: Same as from the buttons, but guarantee LDR and AM/PM LED works
#define BUZZER   33
#define LDR      34
#define AMPM_LED 32

//TIME
int hour=12, minute=0, second=0;
bool isAM=true;

int month=1, day=1, year=2026;

//CLOCK
unsigned long lastSecondTick=0;
bool colonState=false;

//DISPLAY MODE
enum DisplayMode {TIME_MODE, YEAR_MODE};
DisplayMode displayMode = TIME_MODE;

//MODE BUTTON STATE
unsigned long pressStart=0;
unsigned long lastClickTime=0;
int clickCount=0;

//TIME SOURCE(RTC or NTP server) 
enum TimeSource {SERVER, RTC_MANUAL};
TimeSource timeSource = SERVER;

bool timeReceived=false;

//Buzzer beeps
void beep(int count){
  for(int i=0;i<count;i++){
    digitalWrite(BUZZER,HIGH);
    delay(120);
    digitalWrite(BUZZER,LOW);
    delay(120);
  }
}

//SELF BOOT TEST
//(Adjust the "10000" if you need lower boot time test, default is 10seconds test run) 
void bootSelfTest(){

  unsigned long start = millis();

  while(millis() - start < 10000){

    uint8_t segs[4];

    for(int i=0;i<4;i++)
      segs[i]=random(0,128);

    display.setSegments(segs);
    dateDisplay.setSegments(segs);

    digitalWrite(AMPM_LED,!digitalRead(AMPM_LED));

    digitalWrite(BUZZER,HIGH);
    delay(20);
    digitalWrite(BUZZER,LOW);

    delay(200);
  }

  display.clear();
  dateDisplay.clear();
}

//WIFI
void connectWiFi(){

  WiFi.setSleep(false);
  WiFi.begin(ssid,password);

  unsigned long start=millis();

  while(WiFi.status()!=WL_CONNECTED && millis()-start<10000)
    delay(500);

  if(WiFi.status()==WL_CONNECTED)
    configTime(8*3600,0,ntpServer);//change this depending on your UTC, default PHI UTC+8
}

//NTP server
void updateFromNTP(){

  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)) return;

  int h=timeinfo.tm_hour;

  minute=timeinfo.tm_min;
  second=timeinfo.tm_sec;

  lastSecondTick = millis();

  if(h>=12){
    isAM=false;
    if(h>12) h-=12;
  }else{
    isAM=true;
    if(h==0) h=12;
  }

  hour=h;

  day=timeinfo.tm_mday;
  month=timeinfo.tm_mon+1;
  year=timeinfo.tm_year+1900;

  timeReceived=true;
}

//CLOCK TICK(This uses the ESP32 internal counter) 
void internalClockTick(){

  second++;

  if(second>=60){
    second=0;
    minute++;

    if(minute>=60){
      minute=0;
      hour++;

      if(hour==12 && minute==0){
        if(isAM) beep(3);
        else beep(2);
      }

      if(hour>12){
        hour=1;
        isAM=!isAM;
      }
    }
  }
}

//DISPLAYS
void updateDisplay(){

  int dateValue = month*100 + day;
  dateDisplay.showNumberDecEx(dateValue,0x40,true);

  if(displayMode==YEAR_MODE){
    display.showNumberDec(year,true);
    return;
  }

  int value=hour*100+minute;
  uint8_t dots = colonState ? 0x40 : 0;

  display.showNumberDecEx(value,dots,true);
}

// ===== LDR BRIGHTNESS =====
void updateBrightness(){

  int light=analogRead(LDR);

  int brightness=1;

  if(light>3000) brightness=7;
  else if(light>2000) brightness=5;
  else if(light>1000) brightness=3;
  else brightness=1;

  display.setBrightness(brightness);
  dateDisplay.setBrightness(brightness);
}

//MODE BUTTON CONFIG
void handleModeButton(){

  if(!digitalRead(BTN_MODE)){

    pressStart=millis();

    while(!digitalRead(BTN_MODE));

    unsigned long pressLen = millis()-pressStart;

    if(pressLen>2000){
      displayMode = TIME_MODE;
      return;
    }

    if(millis()-lastClickTime<400)
      clickCount++;
    else
      clickCount=1;

    lastClickTime=millis();

    if(clickCount==2)
      displayMode = YEAR_MODE;
  }
}

//TIME SOURCE BUTTON CONFIG
void handleSourceButton(){

  if(!digitalRead(BTN_SOURCE)){
    delay(300);

    if(timeSource==SERVER)
      timeSource=RTC_MANUAL;
    else
      timeSource=SERVER;
  }
}

//THE SETUP SECTION
void setup(){

  pinMode(BUZZER,OUTPUT);
  pinMode(AMPM_LED,OUTPUT);

  pinMode(BTN_MODE,INPUT_PULLUP);
  pinMode(BTN_ADJUST,INPUT_PULLUP);
  pinMode(BTN_SET,INPUT_PULLUP);
  pinMode(BTN_LEFT,INPUT_PULLUP);
  pinMode(BTN_RIGHT,INPUT_PULLUP);
  pinMode(BTN_SOURCE,INPUT_PULLUP);

  display.setBrightness(4);
  dateDisplay.setBrightness(4);

  bootSelfTest();

  connectWiFi();

  updateFromNTP();   // immediate sync
}

//THE LOOP SECTION
void loop(){

  if(millis()-lastSecondTick>=1000){

    lastSecondTick+=1000;

    colonState=!colonState;

    internalClockTick();
  }

  if(WiFi.status()!=WL_CONNECTED)
    WiFi.reconnect();

  //THIS WILL RETRY NTP SERVER RECONNECTION IF IT FAILS
  if(!timeReceived && WiFi.status()==WL_CONNECTED)
    updateFromNTP();

  //PERIODIC RESYNC, to correct time drift
  if(timeReceived && millis()-lastNtpSync > NTP_INTERVAL){

    updateFromNTP();
    lastNtpSync=millis();
  }

  digitalWrite(AMPM_LED,isAM);

  updateBrightness();

  handleModeButton();
  handleSourceButton();

  updateDisplay();
}

//For additional info, read the README file

//Note from the creator: "I sincerely apologize if some doesn't work, but do not worry as I will update the code as I made it work, hence, enjoy your making! 
