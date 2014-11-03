/**
Kotibussi

Apps4Finland consept demo

Copyright Kaj-Michael Lang <milang@tal.org>
GPLv2
*/

/*
 * LCD display
 *
 *   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 
 * 0 B P : n n n n     N :  m  m  :  s  s
 * 1 B S : x x x       S :  m  m  :  s  s
 * 2
 * 3
 */

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <RTClib.h>

// Use W5200 based ethernet shield
#include <EthernetV2_0.h>
// MQTT
#include <PubSubClient.h>

#define BACKLIGHT_PIN     3

// #define SDCARD_CS 4 // Not used

#define ETH_RESET 8 // W5200 Powerdown pin
#define ETH_POWER 9 // W5200 Reset pin
#define ETH_INT 4 // W5200 Int pin

// Control
#define JX A0
#define JY A1
#define JB 7

// Adjust if needed
#define LCD_COLS 20
#define LCD_ROWS 4

#define MQTT_SERVER "192.168.1.249"

byte mac[]    = {  0xDA, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
byte server[] = { 192, 168, 1, 249 };

// My IP
IPAddress ip(192,168,1,177);

// Default gateway
IPAddress gw(192,168,1,254);

int buss_stop=1170;
char *buss_line="22";

int screen=0;

String buss_now;
String buss_next;

// Topic buffer
char tbuf[32];

int nowsec=-1;
int nextsec=-1;

bool needs_update=true;
bool needDirection=false;
bool btnstatus=false;

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7);
EthernetClient ethClient;

RTC_DS1307 rtc;
DateTime now;
unsigned long currentTime;
unsigned long loopTime;

void callback(char* topic, byte* payload, unsigned int length) {
  String tmp;
  String st(topic);
  
  payload[length]=0;
  tmp+=(char *)payload;
   
  if (st==buss_now) {
    nowsec=tmp.toInt();
  } else if (st==buss_next) {
    nextsec=tmp.toInt();
  }
  
  needs_update=true;    
}

PubSubClient client("192.168.1.249", 1883, callback, ethClient);

void printZeroPadded(int n) {
  if (n<10)
    lcd.print("0");
  lcd.print(n);
}

void updateTimes() {
  printTimeAt(11,0,nowsec);
  printTimeAt(11,1,nextsec);
}

void updateLines() {
  lcd.setCursor(0,0);
  lcd.print("BS:");
  lcd.print(buss_stop);
  lcd.setCursor(0,1);
  lcd.print("L:");
  lcd.print(buss_line);
  lcd.setCursor(9,0);
  lcd.print("N:");
  lcd.setCursor(9,1);
  lcd.print("S:");
}

void printTimeAt(int c, int r, int t) {
  lcd.setCursor(c,r);
  if (t<0) {
    lcd.print("--:--");
    return;
  }  
  
  int m=t/60;
  int s=t-(m*60);

  printZeroPadded(m);
  lcd.print(":");
  printZeroPadded(s);
}

void setupEthernet() {
  pinMode(ETH_POWER, OUTPUT);
  pinMode(ETH_RESET, OUTPUT);
  pinMode(ETH_INT, INPUT); 
  digitalWrite(ETH_POWER, LOW);  //enable power
  
  digitalWrite(ETH_RESET,LOW); // Reset W5200
  delay(10); // min time 2us 
  digitalWrite(ETH_RESET,HIGH); // Bring out of reset
  delay(200); // min wait time is 150ms
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");

  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(BACKLIGHT_ON); 
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.clear();
  lcd.print("TKB-AVSDemo");
  
  Wire.begin();
  rtc.begin();
  
#ifdef SDCARD_CS
  pinMode(SDCARD_CS,OUTPUT);
  digitalWrite(SDCARD_CS,HIGH);//Deselect the SD card
#endif  
  
  // SPI into master mode
  pinMode(10, OUTPUT);
  // Enable LED
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  // Button input
  pinMode(JB, INPUT); 
 
  setupEthernet();
  Ethernet.begin(mac, ip, gw, gw);
  
  delay(1000);
  digitalWrite(13, HIGH);
  connectToServer();  
  delay(500);
  lcd.clear();
  digitalWrite(13, HIGH);
  updateLines();
  currentTime = millis();
  loopTime = currentTime; 
}

void connectToServer() {
  do {
    delay(500);
    digitalWrite(13, LOW);
    lcd.clear();
    lcd.print("Connecting...");
    Serial.print(".");
  } while(connectMQTT()<0);
}

int connectMQTT() {
  if (client.connect("talorg-kb")) {
    lcd.print("OK");
    prepareTopics(buss_stop);
    subscribeBusStop();
    return 0;
  }
  lcd.print("ER");
  return -1;  
}

void joystickRead() {
  int jx,jy,btn;
  
  btn = !digitalRead(JB);
  
  if (needDirection) {
    jx = analogRead(JX);
    jy = analogRead(JY);
  }  
  
  if (btn==1 && btnstatus==false) {
    screen++;
    if (screen>2)
      screen=0;
    prepareScreen();
    btnstatus=true;
  } else if (btn==0)
    btnstatus=false;
}

void prepareTopics(int bs) {
  buss_now="stop/";
  buss_now+=bs;
  
  buss_next=buss_now;
  buss_next+="/next";
}

void subscribeBusStop() {
  buss_now.toCharArray(tbuf, 32);
  client.subscribe(tbuf);
  
  buss_next.toCharArray(tbuf, 32);
  client.subscribe(tbuf);
}

void unSubscribeBusStop() {
}

void printTimeAt(int c, int r, int h, int m, int s) {
  lcd.setCursor(c,r);
  printZeroPadded(h);
  lcd.print(":");  
  printZeroPadded(m);
  lcd.print(":");
  printZeroPadded(s);
}

void displayDateScreen() {  
  printTimeAt(LCD_COLS/2-8/2, 0, now.hour(), now.minute(), now.second());
  printTimeAt(LCD_COLS/2-10/2, 1, now.day(), now.month(), now.year());
}

void displayBusScreen() {
  if (needs_update) {
    updateTimes();
    needs_update=false;
  }
}

void displayScreen() {
  switch (screen) {
    case 0: // Default
      displayBusScreen();
    break;
    case 1: // Date & Time
      displayDateScreen();
    break;
    case 2: // ?
    
    break;
    case 3: // Settings screen ?
    
    break;
  }
}

void prepareScreen() {
  lcd.clear(); 
  switch (screen) {
    case 0: // Default
      updateLines();
      needs_update=true;
    break;
  }
}

void cronEverySecondOnly() {
  currentTime = millis();
  if (currentTime < (loopTime + 1000))
    return;
    
  now = rtc.now();  
  loopTime = currentTime;
}

void loop() {
  client.loop();
  joystickRead();
  displayScreen();
}
