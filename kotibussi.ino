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
  lcd.clear();

  Serial.println(topic);
  Serial.println(length);
  Serial.println((char *)payload);
  
  String tmp; 
  payload[length]=0;
  tmp+=(char *)payload;
  
  if (strcmp(topic, buss_now)) {
    nowsec=tmp.toInt();
  } else if (strcmp(topic, buss_next)) {
    nextsec=tmp.toInt();
  }

  Serial.println("NS");
  Serial.println(nowsec);
  Serial.println("NXT");
  Serial.println(nextsec);
  Serial.println("----");
    
  updateLCD();  
}

void updateLCD() {
  lcd.setCursor(0,0);
  lcd.print("BS:");
  lcd.print(buss_stop);

  lcd.setCursor(0,1);
  lcd.print("L:");
  lcd.print(buss_line);
  
  lcd.setCursor(8,0);
  lcd.print("N:");
  printTimeAt(10,0,nowsec);
  
  lcd.setCursor(8,1);
  lcd.print("S:");
  printTimeAt(10,1,nextsec);
}

void printTimeAt(int c, int r, int t) {
  lcd.setCursor(c,r);
  if (t<0) {
    lcd.print("--:--");
    return;
  }  
  
  int m=t/60;
  int s=t-(m*60);

  Serial.println("***");
  Serial.println(m);  
  Serial.println(s);
  Serial.println("***");
  
  if (m<10)
    lcd.print("0");
  lcd.print(m);
  lcd.print(":");
  if (s<10)
    lcd.print("0");
  lcd.print(s);
}

PubSubClient client(server, 1883, callback, ethClient);

void setup() {
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(BACKLIGHT_ON); 
  lcd.begin(16,2);
  lcd.clear();
  lcd.print("TKB-AVSDemo");  
  delay(1000);
  
  Serial.begin(9600);
  Serial.println();
  
  Ethernet.begin(mac, ip);
  
  lcd.clear();
  
  Serial.println("C");
  if (client.connect("demo-")) {
    // client.publish("test","hello world");
    client.subscribe(buss_now);
    client.subscribe(buss_next);
    lcd.print("OK");
    Serial.println("OK");
  } else {
    lcd.print("Er");
    Serial.println("F");
  }
  delay(1000);
}

void loop() {  
  client.loop();
}
