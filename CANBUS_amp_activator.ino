/*
  2018 Copyright (c) Roman Valeev. 
  Author:Roman Valeev
  Contacts:
    baralgin1003@yandex.ru 
*/

/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include <mcp_can.h>
#include <SPI.h>
#include <EEPROM.h>
/*End of auto generated code by Atmel studio */

MCP_CAN CAN0(10);     // Set CS to pin 10
#define CAN0_INT 2

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

long unsigned int rxId;
unsigned char len = 0;            // length of received data
unsigned char rxBuf[8];           // Array of received data
char msgString[128];              // Array to store serial string

void setup() {
  delay(1000);
  Serial.begin(38400);
  pinMode(CAN0_INT, INPUT);
  delay(500);
  
  //reading stored settings
  byte stnTmp[8];
  int sumTmp;
  for (int i = 0; i < 8; i++) {
    stnTmp[i] = EEPROM.read(i);
    sumTmp = sumTmp + EEPROM.read(i);
  }
  
  //if empty then do nothing
  Serial.println("summ eeprom= " + String(sumTmp));
  if (sumTmp != 388 && sumTmp != 2048) {
    Serial.println(F("loaded from eeprom"));
    memccpy(forSend, stnTmp, "", 8);
    for (int i = 0; i < sizeof forSend; i++) {
      Serial.println(forSend[i], HEX);
    }
  } else {
    Serial.println(F("not loaded from eeprom"));
  }

  if (CAN0.begin(MCP_ANY, CAN_100KBPS, MCP_8MHZ) == CAN_OK) Serial.println(F("MCP2515 Initialized Successfully!"));
  else Serial.println(F("Error Initializing MCP2515..."));

  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
}

void sendOutTemp(byte inp) {
  //2e 27 01 11 c6 (ид.команда.длина.данные.чек)
  byte out = ~((0x27 + (0x1 - inp / 2)));

  byte temp[5];
  temp[0] = 0x2e;
  temp[1] = 0x27;
  temp[2] = 0x01;
  temp[3] = out;
  int sum = 0;
  for (int i = 1; i < 4; i++) {
    sum += (int)temp[i];
  }
  temp[4] = ((sum & 255) ^ 255);
  Serial.write(temp, 5);
}

void sendCAN() {
  //switch on amp
  byte a[] = {0xFC, 0xFF, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  CAN0.sendMsgBuf(0x009, 0, 8, a);

  byte sndStat = CAN0.sendMsgBuf(addr, 0, 8, forSend);
  delay(18);
  CAN0.sendMsgBuf(addr, 0, 8, fff);
  delay(18);
  CAN0.sendMsgBuf(addr, 0, 8, fff);
  delay(18);
  CAN0.sendMsgBuf(addr, 0, 8, fff);

  delay(200);   // send data per 100ms
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

    Serial.println(F("Chsumm OK"));
    sendCAN();
  } else {
    Serial.println(F("Chsumm fail"));
  }

  return ret;
}



void loop() {
  //receive CAN temp
  if (!digitalRead(CAN0_INT))
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    if (rxId == 0x531) {
      sendOutTemp(rxBuf[0]);//-25гр
    }
  }


  if (Serial.available())
  {
    Serial.setTimeout(20);
    Serial.readBytesUntil(77, receive, 10);
    checkSumm();
  }

  if (millis() - LastTime > interval) {  // проверяем, прошла ли уже секунда с момента последнего выполнения условия;
    LastTime = millis();
    sendCAN();
  }

}
