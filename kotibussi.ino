/**
Kotibussi

Apps4Finland consept demo

Copyright Kaj-Michael Lang <milang@tal.org>
GPLv2
*/

/*
 * LCD display
 *
 *   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 
 * 0 B P : n n n n     N :  m  m  :  s  s
 * 1 B S : x x x       S :  m  m  :  s  s
 */

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <ethernet_comp.h>
#include <UIPServer.h>
#include <Dhcp.h>
#include <Dns.h>
#include <UIPUdp.h>
#include <UIPClient.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>

#define BACKLIGHT_PIN     3

byte mac[]    = {  0xDA, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
byte server[] = { 10, 1, 1, 1 };
byte ip[]     = { 10, 1, 1, 2 };

char *buss_now="stop/1170";
char *buss_next="stop/1170/next";
int buss_stop=1170;
char *buss_line="22";

int nowsec=-1;
int nextsec=-1;

LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7);
EthernetClient ethClient;


void callback(char* topic, byte* payload, unsigned int length) {
  String tmp; 
  payload[length]=0;
  tmp+=(char *)payload;
  
  if (strcmp(topic, buss_now)==0) {
    nowsec=tmp.toInt();
  } else if (strcmp(topic, buss_next)==0) {
    nextsec=tmp.toInt();
  }
    
  updateTimes();  
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

  if (m<10)
    lcd.print("0");
  lcd.print(m);
  lcd.print(":");
  if (s<10)
    lcd.print("0");
  lcd.print(s);
}

PubSubClient client(server, 1883, callback, ethClient);

int connectMQTT() {
  if (client.connect("demo-")) {
    // client.publish("test","hello world");
    client.subscribe(buss_now);
    client.subscribe(buss_next);
    lcd.print("OK");
    return 0;
  }
  lcd.print("ER");
  return -1;  
}

void setup() {
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(BACKLIGHT_ON); 
  lcd.begin(16,2);
  lcd.clear();
  lcd.print("TKB-AVSDemo");  
  
  Serial.begin(9600);
  Serial.println();
  
  Ethernet.begin(mac, ip);  
  
  do {
    delay(500);
    lcd.clear();
    lcd.print("Connecting...");
    Serial.println("C");
  } while(connectMQTT()<0);
  delay(500);
  lcd.clear();
  updateLines();
}

void loop() {  
  client.loop();
}
