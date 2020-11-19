#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include "DHT.h"
#include <BluetoothSerial.h>

//#define LED1 16
//#define LED2 17
//#define LED3 4
#define LED_PIN 2
#define LED1 16
#define LED_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8
#define BUTTON_PIN 15
#define RESET_EEPROM false
#define DHTPIN 4

unsigned char writePage[256];
unsigned char readPage[256];
char datakit[256];
char setData[256];


#define BH1750_ADDRESS 0x23
#define BHI1750_DATALEN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
BluetoothSerial SerialBT;

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
void updateDhtData();

bool changeStatus = false;
portMUX_TYPE gpioIntMux = portMUX_INITIALIZER_UNLOCKED;

int dataIndex = 0;
int incomingByte = 0;
String dataSerial;
char myId[32], myPass[32];
float temperature = 0, humidity = 0;
String receivedString;
int lastTransmit = 0;

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
  dht.begin();
  EEPROM.begin(512);
  ledcSetup(LED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, LED_CHANNEL);
  SerialBT.begin("BT Classic");
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

void updateDhtData() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

void loop() {
  bh1750Request(BH1750_ADDRESS);
  if (millis() - lastTransmit > 3000) {
    updateDhtData();
    String temphumData = "Data Sensor ==> Suhu : " + String(temperature) + "°C  dan Kelembapan : " + String(humidity) + " %";
    //SerialBT.println(temphumData);
    Serial.println(temphumData);
    lastTransmit = millis();
  } 
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
  if (SerialBT.available()){
    char receivedChar = SerialBT.read();
    receivedString += receivedChar;
    // on/off led
    if (receivedChar == '!'){
      if (receivedString == "led on!"){
          SerialBT.println("LED Menyala");
          ledOn(LED1);
      } else if (receivedString == "led off!"){
          SerialBT.println("LED Mati");
          ledOff(LED1);
      } else {
          SerialBT.println("Perintah tidak dikenali");
      }

      receivedString = "";
      SerialBT.flush();
    }
    // get temp/humi dht22
    if (receivedString == "TEMP"){
      updateDhtData();
      String temp = String(temperature) + " °C";
      SerialBT.println(temp);
      receivedString = "";
      SerialBT.flush();
    } else if (receivedString == "HUMID"){
      updateDhtData();
      String humid = String(humidity) + " %";
      SerialBT.println(humid);
      receivedString = "";
      SerialBT.flush();
    } else if (receivedString == "LUX"){
      if (bh1750Read(BH1750_ADDRESS) == BHI1750_DATALEN)
      {
        lux = (((unsigned short)buff[0] << 8) | (unsigned short)buff[1])/ 1.2;
        String cahaya = String(lux) + " lux";
        SerialBT.println(cahaya);
      }
      receivedString = "";
      SerialBT.flush();
    } else if (receivedString == "AUTOBRIGHT ON"){
      changeStatus = true;
      receivedString = "";
      SerialBT.flush();
    } else if (receivedString == "AUTOBRIGHT OFF"){
      changeStatus = true;
      receivedString = "";
      SerialBT.flush();
    } 
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
    if (lux > 2500)
    {
      int dutyCycle = 0;
      ledcWrite(LED_CHANNEL, dutyCycle);
    } else if (lux > 0 && lux < 2500)
      {
        int dutyCycle = 255 - (lux/10);
        ledcWrite(LED_CHANNEL, dutyCycle);
      }
  }
  } else if (strcmp(datakit, "Off") == 0)
    {
      if (bh1750Read(BH1750_ADDRESS) == BHI1750_DATALEN)
        {
          lux = (((unsigned short)buff[0] << 8) | (unsigned short)buff[1])/ 1.2;
          Serial.println("Intensitas cahaya : " + String(lux) + " lux");
          int dutyCycle = 255;
          ledcWrite(LED_CHANNEL, dutyCycle);
        }
      //ledOn(LED4);
    }
  delay(1000);
}
void initLed(){
  pinMode(LED1, OUTPUT);
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