#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <../personal_configs/wifi_router.cpp>

#define RX_PIN 26 //Pino 26 RX DO SENSOR!
#define TX_PIN 25 //Pino 25 TX DO SENSOR!

String bufferS = "";
int16_t numero = 0;
unsigned long timer, timerNow = 0;
int valorDoSensor = 0;
/*
Se a conexão inicial transmite em 2400 Bauds, temos então 2400 amostras(bits) por segundo.
Bem como a cada 416us temos 1 bit!
A primeira msg, que é uma msg do sistema é de 1 byte. Ou seja, 8 amostras, 3.33ms. 
*/

boolean send_log = false;
WebSocketsServer webSocket = WebSocketsServer(81);
int counter = 0;

void configSerialMonitor(int num = 9600)
{
  Serial.begin(num);
  delay(1000);
  Serial.println("Monitor working");
}

void connectToRouter(const char *ssid, const char *password)
{
  Serial.print("Conecting to router");

  // https://www.youtube.com/watch?v=B9jJI7p2Gw4&ab_channel=MishMashLabs, setando ip fixo
  // IPAddress local_IP(192, 168, 1, 199);
  // IPAddress gateway(192, 168, 1, 1);
  IPAddress local_IP(192, 168, 0, 199);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 0, 0);
  // if (!WiFi.config(local_IP, gateway, subnet))
  //   Serial.println("STA Failed to configure");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(" .");
  }
  Serial.println("\nConected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void receiveMsg(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  { // switch on the type of information sent
  // case WStype_ERROR:
  //   break;
  // case WStype_BIN:
  //   break;
  // case WStype_FRAGMENT_TEXT_START:
  //   break;
  // case WStype_FRAGMENT_BIN_START:
  //   break;
  // case WStype_FRAGMENT:
  //   break;
  // case WStype_FRAGMENT_FIN:
  //   break;
  case WStype_PING:
    Serial.println("Client " + String(num) + " pinged");
    break;
  case WStype_PONG:
    Serial.println("Client " + String(num) + " ponged");
    break;
  case WStype_DISCONNECTED: // if a client is disconnected, then type == WStype_DISCONNECTED
    Serial.println("Client " + String(num) + " disconnected");
    break;
  case WStype_CONNECTED: // if a client is connected, then type == WStype_CONNECTED
    Serial.println("Client " + String(num) + " connected");
    Serial.println("Server has " + String(webSocket.connectedClients()) + " clients connected");
    Serial.println("Server has " + webSocket.remoteIP(num).toString() + " clients connected");
    // webSocket.enableHeartbeat();
    // webSocket.enableHeartbeat(webSocket.connectedClients());
    break;
  case WStype_TEXT: // if a client has sent data, then type == WStype_TEXT
    char payloadString[strlen((char *)(payload))];

    strcpy(payloadString, (char *)(payload));
    if (strcmp(payloadString, "start logs") == 0)
      send_log = true;
    else if (strcmp(payloadString, "stop logs") == 0)
      send_log = false;
    // connection confirmation
    else if (strcmp(payloadString, "ping") == 0)
      webSocket.sendTXT(num, "pong");
    // get connected ports
    else if (strcmp(payloadString, "ports") == 0)
    {
      // manda as portas que tem senor conectado
      webSocket.sendTXT(num, "{\"connectedPorts\":[\"port1\",\"port2\"]}"); // caso de um sensor de toque e outro de ultrasónico
      // webSocket.sendTXT(num, "{\"connectedPorts\":[]}"); // caso não tenha senosres conectados
    }
    else
    {
      // try to decipher the JSON string received
      // DeserializationError error = deserializeJson(doc_received_payload, payloadString);
      // if (error)
      // {
      //   Serial.print(F("deserializeJson() failed: "));
      //   Serial.println(error.f_str());
      //   break;
      // }
      // else
      // {
      //   // JSON string was received correctly, so information can be retrieved:
      //   const char *cmd = doc_received_payload["cmd"];
      //   Serial.println("Received json from wsClient: " + String(num));
      //   Serial.println("cmd: " + String(cmd));
      //   if (strcmp(cmd, "port config") == 0)
      //   {
      //     const char *portName = doc_received_payload["portName"];
      //     const char *sensorType = doc_received_payload["sensorType"];
      //     Serial.println("portName: " + String(portName));
      //     Serial.println("sensorType: " + String(sensorType));
      //   }
      //   Serial.println("");
      // }
    }

    Serial.printf("received: payload [%u]: %s\n", num, payloadString);
    // try to decipher the JSON string received
    // DeserializationError error = deserializeJson(doc_rx, payload);
    // if (error) {
    //   Serial.print(F("deserializeJson() failed: "));
    //   Serial.println(error.f_str());
    //   return;
    // }
    // else {
    //   // JSON string was received correctly, so information can be retrieved:
    //   const char* g_brand = doc_rx["brand"];
    //   const char* g_type = doc_rx["type"];
    //   const int g_year = doc_rx["year"];
    //   const char* g_color = doc_rx["color"];
    //   Serial.println("Received guitar info from user: " + String(num));
    //   Serial.println("Brand: " + String(g_brand));
    //   Serial.println("Type: " + String(g_type));
    //   Serial.println("Year: " + String(g_year));
    //   Serial.println("Color: " + String(g_color));
    // }
    // Serial.println("");
    break;
  };
}

void setup() {
  configSerialMonitor(115200);
  connectToRouter(ROUTER_NAME, ROUTER_PASS);  
  delay(1000);

  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, INPUT);  
  delay(1000);

  webSocket.begin();
  webSocket.onEvent(receiveMsg);
  Serial.print("Configurado");
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

void lendoDados(){  
  if (estadoAltoRepouso()){
    while(1){
      if(!digitalRead(TX_PIN)){
        ets_delay_us(5);
        int i = 0;
        int k = 0;
        bufferS = "";
        numero = 0;

        while(i < 40){        
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
            bufferS += '1';
          }
          else {
            bufferS += '0';
          }        
          i++;  
          ets_delay_us(14); 
        }

        int incremento =  0;
        for (int i = 11; i < 19; i++)  {
          if (bufferS[i] == '1'){
            numero |= 1 << incremento;
          }
          incremento++;
        }
        for (int i = 21; i < 29; i++)  {
          if (bufferS[i] == '1'){
            numero |= 1 << incremento;
          }
          incremento++;
        }
        if(numero >= 0){
          valorDoSensor = numero;
        }  
        break;
      }
    }
  }
  return;
}

int logTimerSimulator = 0;
void enviandoDados(){
  webSocket.loop();
  int bufferDelay = 20;
  if (send_log && webSocket.connectedClients())
  {
    String logs = "{\"logs\":{\"port1\":[";
    for (int i = 0; i < bufferDelay; i++) {
      if (i == bufferDelay - 1) logs.concat("{\"value\":" + String(random(10)) + ",\"time\":" + String(logTimerSimulator) + "}");
      else logs.concat("{\"value\":" + String(random(10)) + ",\"time\":" + String(logTimerSimulator) + "},");
      logTimerSimulator++;
    }
    logTimerSimulator -= bufferDelay;
    logs.concat("],\"port2\":[");
    for (int i = 0; i < bufferDelay; i++) {
      if (i == bufferDelay - 1) logs.concat("{\"value\":" + String(random(10)) + ",\"time\":" + String(logTimerSimulator) + "}");
      else logs.concat("{\"value\":" + String(random(10)) + ",\"time\":" + String(logTimerSimulator) + "},");
      logTimerSimulator++;
    }
    logs.concat("]}}");
    webSocket.broadcastTXT(logs);
    counter++;
    delay(bufferDelay);
  } else {
    logTimerSimulator = 0;
  }
}


void loop() {
  // lendoDados();  
  enviandoDados();
  // Serial.println(valorDoSensor);
}