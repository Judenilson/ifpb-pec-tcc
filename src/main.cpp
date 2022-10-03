#include <Arduino.h>

#define RX_PIN 26 //Pino 26 RX DO SENSOR!
#define TX_PIN 25 //Pino 25 TX DO SENSOR!

uint32_t buffer = 0;
String bufferS = "";
unsigned long timer, timerNow = 0;
/*
Se a conexão inicial transmite em 2400 Bauds, temos então 2400 amostras(bits) por segundo.
Bem como a cada 416us temos 1 bit!
A primeira msg, que é uma msg do sistema é de 1 byte. Ou seja, 8 amostras, 3.33ms. 
*/

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, INPUT);  
  delay(1000);
  Serial.println("Iniciando comunicacao serial"); 
}

void lendoDados(){  
  // timer = micros();  
  ets_delay_us(5);
  int i = 0;
  buffer = 0;
  bufferS = "";

  while(i < 40){    
    
    // timer = micros();  
    int j = 0;
    int bit = 0;
    while(j <= 9){
      if (digitalRead(TX_PIN) == 1) {
        bit++;
      } else {
        bit--;
      };
      j++;
    }
    if (bit > 0){
      // buffer |= 1 << i;
      bufferS += '1';
    }
    else {
      bufferS += '0';
    }
    i++;  

    // timerNow = micros();    
    // Serial.print("Tempo: ");
    // Serial.println(timerNow-timer);

    // if ( (i==0) || (i==9) || (i==10) || (i==19) || (i==20) || (i==29) || (i==30) || (i==39) ){        
    //   i++;
    // } else {
    //   buffer |= digitalRead(TX_PIN) << j;
    //   j++;
    //   i++;
    // }
    ets_delay_us(14); 
  }
  Serial.println(bufferS);
  return;
}

int estadoAltoRepouso(){
  /*
  Leitura do estado de repouso, antes do envio de dados.
  Nesse estado o sinal está em 1 constantemente até cair para 0 quando se dá o início de envio.
  Caso leia uma sequência de 10 bits 1, significa que quando cair para 0, de fato é dados e não ruído.
  */
  ets_delay_us(5);
  int count = 0;
  while(1){      
    if (digitalRead(TX_PIN)){        
      count ++;
    } else {
      count = 0;
    }
    if (count >= 10){
      return 1;
    }
    ets_delay_us(14); 
  }
  return 0;
}

void loop() {
  if (estadoAltoRepouso()){
    while(1){
      if(!digitalRead(TX_PIN)){
        lendoDados();  
        break;
      }
    }
  }
}