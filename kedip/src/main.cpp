#include <Arduino.h>

#define LED1 16
#define LED2 17
#define LED3 18
#define LED4 19

void setup() {
  Serial.begin(9600);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  delay(1000);
}

void loop() {
  if (Serial.available()){
    char angka = Serial.read();
    if (angka == '1'){
      Serial.print("Karakter yang di kirim :");
      Serial.println(angka);
      digitalWrite(LED1, HIGH);
      Serial.println("LED1 On");
      delay(2000);
      digitalWrite(LED1, LOW);
      Serial.println("LED1 Off");
    } else if (angka == '2'){
        Serial.print("Karakter yang di kirim :");
        Serial.println(angka);
        digitalWrite(LED2, HIGH);
        Serial.println("LED2 On");
        delay(2000);
        digitalWrite(LED2, LOW);
        Serial.println("LED2 Off");
    } else if (angka == '3'){
        Serial.print("Karakter yang di kirim :");
        Serial.println(angka);
        digitalWrite(LED3, HIGH);
        Serial.println("LED3 On");
        delay(2000);
        digitalWrite(LED3, LOW);
        Serial.println("LED3 Off");
    } else if (angka == '4'){
        Serial.print("Karakter yang di kirim :");
        Serial.println(angka);
        digitalWrite(LED4, HIGH);
        Serial.println("LED4 On");
        delay(2000);
        digitalWrite(LED4, LOW);
        Serial.println("LED4 Off");
    }
  }
}