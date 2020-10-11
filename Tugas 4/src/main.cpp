#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

#define LED1 16
#define LED2 17
#define LED3 4
#define LED4 2
#define BUTTON_PIN 15
#define RESET_EEPROM false

unsigned char writePage[256];
unsigned char readPage[256];
char datakit[256];
char setData[256];


#define BH1750_ADDRESS 0x23
#define BHI1750_DATALEN 2

void bh1750Request(int address);
int bh1750Read(int address);
void bacaEeprom(int address);
void tulisEeprom(int address, int EEPROM_SIZE, char * data);
void tulisPWifi(int address, int EEPROM_SIZE, char * data);
void bacaPWifi(int address);
String getValue(String data, char separator, int index);
void initLed();
void ledOn(int nomorLed);
void ledOff(int nomorLed);
void state();

bool changeStatus = false;
portMUX_TYPE gpioIntMux = portMUX_INITIALIZER_UNLOCKED;

int dataIndex = 0;
int incomingByte = 0;
String dataSerial;
char myId[32], myPass[32];

byte buff[2];

unsigned short lux = 0;

void IRAM_ATTR gpioISR(){
  portENTER_CRITICAL(&gpioIntMux);
  changeStatus = true;
  portEXIT_CRITICAL(&gpioIntMux);
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  EEPROM.begin(512);
  if ( RESET_EEPROM ) {
    for (int i = 0; i < 512; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
    delay(500);
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, &gpioISR, FALLING);
  Serial.println();
  bacaEeprom(0);
  if (strcmp(datakit, "On") == 0){
    Serial.print("Status autobrightness On");
  } else if(strcmp(datakit, "Off") == 0) {
    Serial.print("Status autobrightness Off");
  }
  Serial.println();
  Serial.println("Data Parameter WIFI :");
  bacaPWifi(32);
  Serial.print("SSID : ");
  Serial.println(setData);
  bacaPWifi(64);
  Serial.print("Password : ");
  Serial.println(setData);
  initLed();
  // put your setup code here, to run once:
}

void loop() {
  bh1750Request(BH1750_ADDRESS);
  
   while(Serial.available()) {
      dataSerial = Serial.readString();
      Serial.print("Menerima Parameter setting WIFI : ");// read the incoming data as string
      Serial.println(dataSerial);
      Serial.println("Start Write WIFI setting ..........");
      String id = getValue(dataSerial,';',0);
      String pass = getValue(dataSerial,';',1);
      int len_id = id.length();
      int len_pass = pass.length();
      id.toCharArray(myId, len_id+1);
      pass.toCharArray(myPass, len_pass+1);
      tulisPWifi(32, len_id, myId);
      tulisPWifi(64, len_pass, myPass);
      delay(1000);
    }
 
   //menunggu interruption
  if (changeStatus){
    portENTER_CRITICAL(&gpioIntMux);
    changeStatus = false;
    portEXIT_CRITICAL(&gpioIntMux);
    state();
    bacaEeprom(0);
  }
  //cek data status bright mode
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
      Serial.println("Semua led menyala");
    }
  delay(1000);
  
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

void tulisEeprom(int address, int EEPROM_SIZE, char * data){
  for (int i = 0; i < EEPROM_SIZE; i++){
    EEPROM.write(address + i, data[i]);
  }
  EEPROM.commit();
}

void bacaEeprom(int address)
{
  for (int i = 0; i < 4; i++)
  {
     datakit[i] = EEPROM.read(address + i);
  }
}
void tulisPWifi(int address, int EEPROM_SIZE, char * data){
  //reset eeprom address 31 dan 64 replace data baru
  for (int i = 0; i < 31; i++) {
      EEPROM.write(address+i, 0);
  }
  //new data parameter wifi
  for (int i = 0; i < EEPROM_SIZE; i++){
    EEPROM.write(address + i, data[i]);
  }
  EEPROM.commit();
}

void bacaPWifi(int address)
{
  for (int i = 0; i < 256; i++)
  {
     setData[i] = EEPROM.read(address + i);
  }
}
void state(){
    if (strcmp(datakit, "On") == 0){
      char status[4] = "Off";
      tulisEeprom(0,sizeof(status),status);
      Serial.print("Status autobrightness Off");
      Serial.println();
    } else if (strcmp(datakit, "Off") == 0) {
      char status[3] = "On";
      tulisEeprom(0,sizeof(status),status);
      Serial.print("Status autobrightness On");
      Serial.println();
    }
    delay(1000);
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

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}