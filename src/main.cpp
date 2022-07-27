#include <Arduino.h>

#define RX_PIN 26 //Pino 26
#define TX_PIN 10 //Pino D3
int sensorValue = 0;
int frequency = 0;
int speedCommunicationSensor = 2400; //BAUDS
String tenBitsWord = "";
int countBits = 0;  
bool flag = true;
int countBytes = 0;
unsigned long timer, timerNow = 0;
/*
Se a conexão inicial transmite em 2400 Bauds, temos então 2400 amostras(bits) por segundo.
Bem como a cada 416us temos 1 bit!
A primeira msg, que é uma msg do sistema é de 1 byte. Ou seja, 8 amostras, 3.33ms. 
*/

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Iniciando comunicacao serial");
  pinMode(RX_PIN, INPUT);
  
  frequency = 1000000/speedCommunicationSensor; //o clock de cada bit! :D
}

void loop() {
  sensorValue = digitalRead(RX_PIN);

  if (sensorValue == 1){
    while (flag) {
      timerNow = micros();
      if (timerNow > (timer+frequency)){
        sensorValue = digitalRead(RX_PIN);
        tenBitsWord += sensorValue;
        countBits ++;
        timer = timerNow;
      }
      if (countBits == 10){ //10 bites, sendo MSB 0 START bit e LSB 1 STOP bit, os 8 bits centrais são dados.
        Serial.println(tenBitsWord);
        tenBitsWord = "";
        countBits = 0;  
        countBytes ++;      
      }
      if (countBytes >= 128){ // quantidade de bytes lidos a princípio para análise de dados, remover!
        flag = false;
      }
    }
  }
}
