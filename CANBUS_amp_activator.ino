/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include <mcp_can.h>
#include <SPI.h>
#include <EEPROM.h>
/*End of auto generated code by Atmel studio */

MCP_CAN CAN0(10);     // Set CS to pin 10

int addr = 0x00f;
byte receive[10];

byte forSend[8] = {10, 10, 10, 10, 10, 10, 255, 255};
byte fff[8] = {255, 255, 255, 255, 255, 255, 255, 255};

long LastTime = 0;
const int interval = 2100;
bool firstTime = true;

int count = 0;
const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN);


void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  //читаем сохраненные настройки
  byte stnTmp[8];
  int sumTmp;
  for (int i = 0; i < 8; i++) {
    stnTmp[i] = EEPROM.read(i);
    sumTmp=sumTmp+EEPROM.read(i);
  }
  //если пусто, то ничего не делаем.
  Serial.println("summ eeprom= " + String(sumTmp));
  if(sumTmp!=388){
     Serial.println("loaded from eeprom");
     memccpy(forSend, stnTmp, "", 8);
  }

  if (CAN0.begin(MCP_ANY, CAN_100KBPS, MCP_8MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);  
}

void sendCAN() {
  //вкл уся
   byte a[] = {0xFC, 0xFF, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    CAN0.sendMsgBuf(0x009, 0, 8, a);

  byte sndStat = CAN0.sendMsgBuf(addr, 0, 8, forSend);
  delay(18);
  CAN0.sendMsgBuf(addr, 0, 8, fff);
  delay(18);
  CAN0.sendMsgBuf(addr, 0, 8, fff);
  delay(18);
  CAN0.sendMsgBuf(addr, 0, 8, fff);

  delay(100);   // send data per 100ms
}

int checkSumm() {
  int ret = 0;
  int CheckSum = 0;
  for (int i = 0; i < sizeof receive - 2; i++) {
    CheckSum = (int) receive[i] + CheckSum;
    // Serial.println((int)receive[i]);
  }
  Serial.println("summ= " + String(CheckSum));
  if (((CheckSum & 255) ^ 255) == (int)receive[8]) {
    ret = 1;
    memccpy(forSend, receive, "", 8);

    for (int i = 0; i < sizeof forSend - 1; i++) {
      Serial.println((int)receive[i]);
      EEPROM.write(i, forSend[i]);
    }

    Serial.println("summ OK");
    sendCAN();
  } else {
    Serial.println("summ fail");
  }

  return ret;
}



void loop() {

  if (Serial.available())
  {
    Serial.readBytesUntil(77, receive, 10);
    checkSumm();
  }

  if (millis() - LastTime > interval) { 
    LastTime = millis();
    sendCAN();
  }

}
