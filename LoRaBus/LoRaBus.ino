/*
  LoRaBus.cpp - Modbus over LoRa

    Copyright (C) 2018-2022 Sfera Labs S.r.l. - All rights reserved.

    For information, see:
    http://www.sferalabs.cc/

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any nlater version.
  See file LICENSE.txt for further informations on licesing terms.
*/

#include <Iono.h>
#include <IonoModbusRtuSlave.h>
#include <LoRa.h>
#include <IonoLoRaNet.h>
#include "SerialConfig.h"
#include "Watchdog.h"

#define DELAY  25

#define ID_NUMBER_GW 0x21
#define ID_NUMBER_SLAVE 0x22

IonoLoRaLocalSlave loRaSlave;
IonoLoRaLocalMaster loRaMaster;
IonoLoRaRemoteSlave slavesBuffer[MAX_SLAVES];
LoRaRemoteSlave *slavesRefsBuffer[MAX_SLAVES];
bool initialized;

void setup() {
  SerialConfig.setup();
  while (!SerialConfig.isConfigured) {
    SerialConfig.process();
  }
  initialized = false;
}

void loop() {
  if (!initialized) {
    initialized = initialize();
    return;
  }
  if (SerialConfig.isGateway) {
    loRaMaster.process();
    if (SerialConfig.isAvailable) {
      SerialConfig.process();
      if (!SerialConfig.isAvailable) {
        startModbus();
      }
      Iono.process();
    } else {
      IonoModbusRtuSlave.process();
    }
  } else {
    loRaSlave.process();
    if (SerialConfig.isAvailable) {
      SerialConfig.process();
    }
  }
  Watchdog.clear();
}

bool initialize() {
  if (SerialConfig.frequency > 0l) {
    if (!LoRa.begin(SerialConfig.frequency * 1000l)) {
      __DEBUGprintln("LoRaBus: initialization failed");
      delay(200);
      return false;
    }
    LoRa.enableCrc();
    LoRa.setSyncWord(0x12);
    LoRa.setSpreadingFactor(SerialConfig.sf);
    LoRa.setTxPower(SerialConfig.txPower);
    LoRaNet.init(SerialConfig.siteId, 3, SerialConfig.pwd);
    LoRaNet.setDutyCycle(SerialConfig.dcWin, SerialConfig.dc);

    if (SerialConfig.isGateway) {
      for (int i = 0; i < MAX_SLAVES; i++) {
        slavesRefsBuffer[i] = &slavesBuffer[i];
      }

      if (SerialConfig.slavesNum > 0) {
        for (int i = 0; i < SerialConfig.slavesNum; i++) {
          slavesRefsBuffer[i]->setAddr(SerialConfig.slavesAddr[i]);
        }
        loRaMaster.setSlaves(slavesRefsBuffer, SerialConfig.slavesNum);
      } else {
        loRaMaster.enableDiscovery(slavesRefsBuffer, MAX_SLAVES);
      }

    } else {
      loRaSlave.setAddr(SerialConfig.address);

      Iono.subscribeDigital(DO1, 0, &IonoLoRaLocalSlave::subscribeCallback);
      Iono.subscribeDigital(DO2, 0, &IonoLoRaLocalSlave::subscribeCallback);
      Iono.subscribeDigital(DO3, 0, &IonoLoRaLocalSlave::subscribeCallback);
      Iono.subscribeDigital(DO4, 0, &IonoLoRaLocalSlave::subscribeCallback);

      Iono.subscribeAnalog(AO1, 0, 0, &IonoLoRaLocalSlave::subscribeCallback);

      subscribeMultimode(SerialConfig.modes[0], DI1, AV1, AI1);
      subscribeMultimode(SerialConfig.modes[1], DI2, AV2, AI2);
      subscribeMultimode(SerialConfig.modes[2], DI3, AV3, AI3);
      subscribeMultimode(SerialConfig.modes[3], DI4, AV4, AI4);
      subscribeMultimode(SerialConfig.modes[4], DI5, 0, 0);
      subscribeMultimode(SerialConfig.modes[5], DI6, 0, 0);

      loRaSlave.setUpdatesInterval(DI1, SerialConfig.inItvl[0]);
      loRaSlave.setUpdatesInterval(DI2, SerialConfig.inItvl[1]);
      loRaSlave.setUpdatesInterval(DI3, SerialConfig.inItvl[2]);
      loRaSlave.setUpdatesInterval(DI4, SerialConfig.inItvl[3]);
      loRaSlave.setUpdatesInterval(DI5, SerialConfig.inItvl[4]);
      loRaSlave.setUpdatesInterval(DI6, SerialConfig.inItvl[5]);
    }
    return true;
  }

  if (SerialConfig.rules[0] != '\0') {
    setLink(SerialConfig.modes[0], SerialConfig.rules[0], DI1, DO1);
    setLink(SerialConfig.modes[1], SerialConfig.rules[1], DI2, DO2);
    setLink(SerialConfig.modes[2], SerialConfig.rules[2], DI3, DO3);
    setLink(SerialConfig.modes[3], SerialConfig.rules[3], DI4, DO4);
  }

  Watchdog.setup();
}

void startModbus() {
  IonoModbusRtuSlave.setInputMode(1, SerialConfig.modes[0]);
  IonoModbusRtuSlave.setInputMode(2, SerialConfig.modes[1]);
  IonoModbusRtuSlave.setInputMode(3, SerialConfig.modes[2]);
  IonoModbusRtuSlave.setInputMode(4, SerialConfig.modes[3]);
  switch (SerialConfig.parity) {
    case 2:
      IonoModbusRtuSlave.begin(0, SPEEDS[SerialConfig.speed], SERIAL_8O1, DELAY);
      break;
    case 3:
      IonoModbusRtuSlave.begin(0, SPEEDS[SerialConfig.speed], SERIAL_8N2, DELAY);
      break;
    default:
      IonoModbusRtuSlave.begin(0, SPEEDS[SerialConfig.speed], SERIAL_8E1, DELAY);
      break;
  }

  IonoModbusRtuSlave.setCustomHandler(&onModbusRequest);
}

void subscribeMultimode(char mode, uint8_t dix, uint8_t avx, uint8_t aix) {
  switch (mode) {
    case 'D':
      Iono.subscribeDigital(dix, DELAY, &IonoLoRaLocalSlave::subscribeCallback);
      break;
    case 'V':
      Iono.subscribeAnalog(avx, DELAY, 0.1, &IonoLoRaLocalSlave::subscribeCallback);
      break;
    case 'I':
      Iono.subscribeAnalog(aix, DELAY, 0.1, &IonoLoRaLocalSlave::subscribeCallback);
      break;
    default:
      break;
  }
}

void setLink(char mode, char rule, uint8_t dix, uint8_t dox) {
  if (mode == 'V' || mode == 'I') {
    return;
  }
  switch (rule) {
    case 'F':
      Iono.linkDiDo(dix, dox, LINK_FOLLOW, DELAY);
      break;
    case 'I':
      Iono.linkDiDo(dix, dox, LINK_INVERT, DELAY);
      break;
    case 'T':
      Iono.linkDiDo(dix, dox, LINK_FLIP_T, DELAY);
      break;
    case 'H':
      Iono.linkDiDo(dix, dox, LINK_FLIP_H, DELAY);
      break;
    case 'L':
      Iono.linkDiDo(dix, dox, LINK_FLIP_L, DELAY);
      break;
    default:
      break;
  }
}

byte onModbusRequest(byte unitAddr, byte function, word regAddr, word qty, byte *data) {
  if (unitAddr == SerialConfig.address) {
    if (function == MB_FC_READ_INPUT_REGISTER && regAddr == 99 && qty == 1) {
      ModbusRtuSlave.responseAddRegister(ID_NUMBER_GW);
      return MB_RESP_OK;
    }
    return MB_RESP_PASS;
  }
  IonoLoRaRemoteSlave *slave = NULL;
  for (int i = 0; i < MAX_SLAVES; i++) {
    if (slavesRefsBuffer[i]->getAddr() == unitAddr) {
      slave = (IonoLoRaRemoteSlave*) slavesRefsBuffer[i];
      break;
    }
  }
  if (slave == NULL) {
    return MB_RESP_IGNORE;
  }

  switch (function) {
    case MB_FC_READ_COILS:
      if (checkAddrRange(regAddr, qty, 1, 4)) {
        for (int i = regAddr; i < regAddr + qty; i++) {
          ModbusRtuSlave.responseAddBit(slave->read(indexToDO(i)) == HIGH);
        }
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_READ_DISCRETE_INPUTS:
      if (checkAddrRange(regAddr, qty, 101, 106)) {
        for (int i = regAddr - 100; i < regAddr - 100 + qty; i++) {
          ModbusRtuSlave.responseAddBit(slave->read(indexToDI(i)) == HIGH);
        }
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_READ_HOLDING_REGISTERS:
      if (regAddr == 601 && qty == 1) {
        ModbusRtuSlave.responseAddRegister(analogToRegister(slave->read(AO1)));
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_READ_INPUT_REGISTER:
      if (checkAddrRange(regAddr, qty, 201, 204)) {
        for (int i = regAddr - 200; i < regAddr - 200 + qty; i++) {
          ModbusRtuSlave.responseAddRegister(analogToRegister(slave->read(indexToAV(i))));
        }
        return MB_RESP_OK;
      }
      if (checkAddrRange(regAddr, qty, 301, 304)) {
        for (int i = regAddr - 300; i < regAddr - 300 + qty; i++) {
          ModbusRtuSlave.responseAddRegister(analogToRegister(slave->read(indexToAI(i))));
        }
        return MB_RESP_OK;
      }
      if (checkAddrRange(regAddr, qty, 1001, 1006)) {
        for (int i = regAddr - 1000; i < regAddr - 1000 + qty; i++) {
          ModbusRtuSlave.responseAddRegister(slave->diCount(indexToDI(i)));
        }
        return MB_RESP_OK;
      }
      if (regAddr == 5001 && qty == 1) {
        ModbusRtuSlave.responseAddRegister(slave->loraRssi());
        return MB_RESP_OK;
      }
      if (regAddr == 5002 && qty == 1) {
        ModbusRtuSlave.responseAddRegister(slave->loraSnr() * 1000);
        return MB_RESP_OK;
      }
      if (regAddr == 5101 && qty == 1) {
        ModbusRtuSlave.responseAddRegister(slave->stateAge());
        return MB_RESP_OK;
      }
      if (regAddr == 99 && qty == 1) {
        ModbusRtuSlave.responseAddRegister(ID_NUMBER_SLAVE);
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_WRITE_SINGLE_COIL:
      if (regAddr >= 1 && regAddr <= 4) {
        bool on = ModbusRtuSlave.getDataCoil(function, data, 0);
        slave->write(indexToDO(regAddr), on ? HIGH : LOW);
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_WRITE_SINGLE_REGISTER:
      if (regAddr == 601) {
        word value = ModbusRtuSlave.getDataRegister(function, data, 0);
        if (value < 0 || value > 10000) {
          return MB_EX_ILLEGAL_DATA_VALUE;
        }
        slave->write(AO1, value / 1000.0);
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    case MB_FC_WRITE_MULTIPLE_COILS:
      if (checkAddrRange(regAddr, qty, 1, 4)) {
        for (int i = regAddr; i < regAddr + qty; i++) {
          bool on = ModbusRtuSlave.getDataCoil(function, data, i - regAddr);
          slave->write(indexToDO(i), on ? HIGH : LOW);
        }
        return MB_RESP_OK;
      }
      return MB_EX_ILLEGAL_DATA_ADDRESS;

    default:
      return MB_EX_ILLEGAL_FUNCTION;
  }
}

bool checkAddrRange(word regAddr, word qty, word min, word max) {
  return regAddr >= min && regAddr <= max && regAddr + qty <= max + 1;
}

word analogToRegister(float val) {
  if (val < 0) {
    return 0xFFFF;
  }
  return val * 1000;
}

uint8_t indexToDO(int i) {
  switch (i) {
    case 1:
      return DO1;
    case 2:
      return DO2;
    case 3:
      return DO3;
    case 4:
      return DO4;
    case 5:
      return DO5;
    case 6:
      return DO6;
  }
}

uint8_t indexToDI(int i) {
  switch (i) {
    case 1:
      return DI1;
    case 2:
      return DI2;
    case 3:
      return DI3;
    case 4:
      return DI4;
    case 5:
      return DI5;
    case 6:
      return DI6;
  }
}

uint8_t indexToAV(int i) {
  switch (i) {
    case 1:
      return AV1;
    case 2:
      return AV2;
    case 3:
      return AV3;
    case 4:
      return AV4;
  }
}

uint8_t indexToAI(int i) {
  switch (i) {
    case 1:
      return AI1;
    case 2:
      return AI2;
    case 3:
      return AI3;
    case 4:
      return AI4;
  }
}
