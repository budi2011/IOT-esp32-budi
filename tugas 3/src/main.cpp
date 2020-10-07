#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <w25q64.hpp>

#define LED1 16
#define LED2 17
#define LED3 4
#define LED4 2
#define BUTTON_PIN 15

unsigned char writePage[256];
unsigned char readPage[256];
char datakit[256];

w25q64 spiChip;

#define BH1750_ADDRESS 0x23
#define BHI1750_DATALEN 2

void bh1750Request(int address);
int bh1750Read(int address);
void bacaW25q64();
void tulisW25q64(char status[]);
void initLed();
void ledOn(int nomorLed);
void ledOff(int nomorLed);
void state();

byte buff[2];
byte chipId[4] = "";

unsigned short lux = 0;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  spiChip.begin();
  spiChip.getId(chipId);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.print("Chip ID in Bytes : ");
  for (int i = 0;i < LEN_ID;i++)
  {
    Serial.print(chipId[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  bacaW25q64();
  if (strcmp(datakit, "On") == 0){
    Serial.print("Status autobrightness On");
  } else {
    Serial.print("Status autobrightness Off");
  }
  Serial.println();
  initLed();
  // put your setup code here, to run once:
}

void loop() {
  bh1750Request(BH1750_ADDRESS);
  state();
  if (strcmp(datakit, "On") == 0)
  {
  if (bh1750Read(BH1750_ADDRESS) == BHI1750_DATALEN)
  {
    lux = (((unsigned short)buff[0] << 8) | (unsigned short)buff[1])/ 1.2;
    Serial.println("Intensitas cahaya : " + String(lux) + " lux");
    if (lux > 0 && lux < 250)
    {
      ledOn(LED4);
      ledOn(LED3);
      ledOn(LED2);
      ledOn(LED1);
    } else if (lux > 250 && lux < 500){
      ledOff(LED4);
      ledOn(LED3);
      ledOn(LED2);
      ledOn(LED1);
    } else if (lux > 500 && lux < 750){
      ledOff(LED4);
      ledOff(LED3);
      ledOn(LED2);
      ledOn(LED1);
    } else if (lux > 750 && lux < 1000){
      ledOff(LED4);
      ledOff(LED3);
      ledOff(LED2);
      ledOn(LED1);
    } else{
      ledOff(LED4);
      ledOff(LED3);
      ledOff(LED2);
      ledOff(LED1);
    }
  }
  } else if (strcmp(datakit, "Off") == 0)
    {
      if (bh1750Read(BH1750_ADDRESS) == BHI1750_DATALEN)
        {
          lux = (((unsigned short)buff[0] << 8) | (unsigned short)buff[1])/ 1.2;
          Serial.println("Intensitas cahaya : " + String(lux) + " lux");
        }
      ledOn(LED4);
      ledOn(LED3);
      ledOn(LED2);
      ledOn(LED1);
    }
  delay(1000);
  // put your main code here, to run repeatedly:
}

void bh1750Request(int address){
  Wire.beginTransmission(address);
  Wire.write(0x10);
  Wire.endTransmission();
}

int bh1750Read(int address)
{
  int i = 0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address,2);
  while (Wire.available())
  {
    buff[i] = Wire.read();
    i++;
  }
  Wire.endTransmission();
  return i;
}

void tulisW25q64(char status[]){
  if (strcmp(status, "On") == 0) {
    memcpy(writePage, "On", sizeof("On"));
    spiChip.erasePageSector(0xFFFF);
    spiChip.pageWrite(writePage, 0xFFFF);
  } else if(strcmp(status, "Off") == 0) {
    memcpy(writePage, "Off", sizeof("Off"));
    spiChip.erasePageSector(0xFFFF);
    spiChip.pageWrite(writePage, 0xFFFF);
  }
  delay(1000);
}

void bacaW25q64()
{
  Serial.println("Read Status autobrightness......");
  spiChip.readPages(readPage, 0xFFFF, 1);
  for (int i = 0; i < 256; i++)
  {
   if (readPage[i] > 7 && readPage[i] < 127)
     datakit[i] = datakit[i] + (char)readPage[i];
  }
}

void state(){
  if (digitalRead(BUTTON_PIN) == LOW){
    if (strcmp(datakit, "On") == 0){
      char status[4] = "Off";
      tulisW25q64(status);
      Serial.print("Status autobrightness Off");
      Serial.println();
    } else if (strcmp(datakit, "Off") == 0) {
      char status[3] = "On";
      tulisW25q64(status);
      Serial.print("Status autobrightness On");
      Serial.println();
    }
    delay(1000);
  }
}

void initLed(){
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
}

void ledOn(int nomorLed)
{
  digitalWrite(nomorLed, HIGH);
}

void ledOff(int nomorLed)
{
  digitalWrite(nomorLed, LOW);
}


