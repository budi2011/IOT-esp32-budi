#include <Arduino.h>

#define LED1 16
#define LED2 17
#define LED3 18
#define LED4 19

const int potPin = 34;
int Nilaipot = 0;
float Nilaiteg = 0;
int Nilaisuhu = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  delay(1000);
}

void loop() {
  Nilaipot = analogRead(potPin);
  Nilaiteg = ((float)Nilaipot / 4095) * 3.3;
  Serial.println("Nilai Tegangan : " + String(Nilaiteg) + " Volt");
  Serial.println("Nilai ADC : " + String(Nilaipot));
  Nilaisuhu = map(Nilaipot, 0, 4095, 0, 100);
  Serial.println("Nilai Suhu : " + String(Nilaisuhu) + "\xC2\xB0" + "C");
  if (Nilaisuhu < 15){
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
  }else if ((Nilaisuhu > 15) and (Nilaisuhu < 25)){
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);
  }else if ((Nilaisuhu > 25) and (Nilaisuhu < 30)){
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    digitalWrite(LED4, LOW);
  }else if (Nilaisuhu > 30){
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, HIGH);
  }
  delay(1000);
}