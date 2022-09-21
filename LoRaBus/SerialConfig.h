/*
  SerialConfig.h

    Copyright (C) 2018-2022 Sfera Labs S.r.l. - All rights reserved.

    For information, see:
    http://www.sferalabs.cc/

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef SerialConfig_h
#define SerialConfig_h

#include <FlashAsEEPROM.h>
#include <FlashStorage.h>
#include "Watchdog.h"

#define MAX_SLAVES  20
#define CONSOLE_TIMEOUT 20000
#define _PORT_USB SERIAL_PORT_MONITOR
#define _PORT_RS485 SERIAL_PORT_HARDWARE

const long SPEEDS[] = {0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

class SerialConfig {
  private:
    static Stream *_port;
    static short _spacesCounter;
    static char _inBuffer[24];

    static void _close();
    static void _enterConsole();
    static void _enterConfigWizard();
    static void _exportConfig();
    static bool _importConfig();
    static bool _consumeWhites();
    template <typename T>
    static void _print(T text);
    static void _readEchoLine(int maxLen, bool returnOnMaxLen,
          bool upperCase, int (*charFilter)(int, int, int, int), int p1, int p2);
    static int _betweenFilter(int c, int idx, int min, int max);
    static int _orFilter(int c, int idx, int p1, int p2);
    static int _modesFilter(int c, int idx, int p1, int p2);
    static int _rulesFilter(int c, int idx, int p1, int p2);
    static void _printConfiguration(byte address, byte speed, byte parity,
        uint32_t frequency, byte txPower, byte sf, uint16_t dc, uint16_t dcWin,
        byte *siteId, byte *pwd, char *modes,
        uint32_t inItvl1, uint32_t inItvl2, uint32_t inItvl3,
        uint32_t inItvl4, uint32_t inItvl5, uint32_t inItvl6,
        char *rules, byte *slavesAddr, byte slavesNum);
    static void _confirmConfiguration(byte address, byte speed, byte parity,
        uint32_t frequency, byte txPower, byte sf, uint16_t dc, uint16_t dcWin,
        byte *siteId, byte *pwd, char *modes,
        uint32_t inItvl1, uint32_t inItvl2, uint32_t inItvl3,
        uint32_t inItvl4, uint32_t inItvl5, uint32_t inItvl6,
        char *rules, byte *slavesAddr, byte slavesNum);
    static bool _readEepromConfig();
    static bool _writeEepromConfig(byte address, byte speed, byte parity,
        uint32_t frequency, byte txPower, byte sf, uint16_t dc, uint16_t dcWin,
        byte *siteId, byte *pwd, char *modes,
        uint32_t inItvl1, uint32_t inItvl2, uint32_t inItvl3,
        uint32_t inItvl4, uint32_t inItvl5, uint32_t inItvl6,
        char *rules, byte *slavesAddr, byte slavesNum);

  public:
    static bool isConfigured;
    static bool isAvailable;
    static bool isGateway;

    static byte address;
    static byte speed;
    static byte parity;
    static uint32_t frequency;
    static byte txPower;
    static byte sf;
    static uint16_t dc;
    static uint16_t dcWin;
    static byte siteId[4];
    static byte pwd[17];
    static char modes[7];
    static uint32_t inItvl[6];
    static char rules[5];
    static byte slavesAddr[MAX_SLAVES];
    static byte slavesNum;

    static void setup();
    static void process();
};

bool SerialConfig::isConfigured = false;
bool SerialConfig::isAvailable = true;
bool SerialConfig::isGateway = false;

Stream *SerialConfig::_port = NULL;
short SerialConfig::_spacesCounter = 0;
char SerialConfig::_inBuffer[24];

byte SerialConfig::address;
byte SerialConfig::speed;
byte SerialConfig::parity;
uint32_t SerialConfig::frequency;
byte SerialConfig::txPower;
byte SerialConfig::sf;
uint16_t SerialConfig::dc;
uint16_t SerialConfig::dcWin;
byte SerialConfig::siteId[4];
byte SerialConfig::pwd[17];
char SerialConfig::modes[7];
uint32_t SerialConfig::inItvl[6];
char SerialConfig::rules[5];
byte SerialConfig::slavesAddr[MAX_SLAVES];
byte SerialConfig::slavesNum;

void SerialConfig::setup() {
  _PORT_USB.begin(9600);
  _PORT_RS485.begin(9600);

  isConfigured = _readEepromConfig();

  if (!isConfigured) {
    address = 1;
    speed = 8;
    parity = 1;
    frequency = 869500l;
    txPower = 14;
    sf = 7;
    dc = 100;
    dcWin = 600;
    siteId[0] = '\0';
    pwd[0] = '\0';
    strncpy(modes, "DDDDDD", 6);
    modes[6] = '\0';
    for (int i = 0; i < 6; i++) {
      inItvl[i] = 0;
    }
    strncpy(rules, "----", 4);
    rules[4] = '\0';
    slavesNum = 0;
  }

  isGateway = (speed >= 1 && speed <= 8);
}

void SerialConfig::process() {
  if (_port == NULL) {
    if (_PORT_USB.available()) {
      _port = &_PORT_USB;
    } else if (_PORT_RS485.available()) {
      _port = &_PORT_RS485;
    }
  }

  if (_port != NULL && _port->available()) {
    int b = _port->read();
    if (b == ' ') {
      if (_spacesCounter >= 4) {
        _enterConsole();
      } else {
        _spacesCounter++;
      }
    } else if (isConfigured) {
      _close();
    } else {
      _port = NULL;
    }
  }

  if (isConfigured && millis() > CONSOLE_TIMEOUT) {
    _close();
  }
}

void SerialConfig::_close() {
  _PORT_USB.end();
  _PORT_RS485.end();
  isAvailable = false;
}

void SerialConfig::_enterConsole() {
  Watchdog.disable();
  delay(100);
  while(_port->read() >= 0) {
    delay(5);
  }
  while (true) {
    _print("=== Sfera Labs - LoRaBus configuration - v1.0 ===\r\n"
           "\r\n    1. Configuration wizard"
           "\r\n    2. Import configuration"
           "\r\n    3. Export configuration"
           "\r\n\r\n> "
         );
    _readEchoLine(1, false, false, &_betweenFilter, '1', '3');
    switch (_inBuffer[0]) {
      case '1':
        _enterConfigWizard();
        break;
      case '2':
        if (!_importConfig()) {
          while(_port->read() >= 0) {
            delay(5);
          }
          _print("\r\nError\r\n\r\n");
        }
        break;
      case '3':
        _exportConfig();
        break;
      default:
        break;
    }
  }
}

bool SerialConfig::_importConfig() {
  byte addressNew = 0;
  byte speedNew = 0;
  byte parityNew = 1;
  uint32_t frequencyNew = 0;
  byte txPowerNew = 0;
  byte sfNew = 0;
  uint16_t dcNew = 0;
  uint16_t dcWinNew = 0;
  byte siteIdNew[4];
  byte pwdNew[17];
  char modesNew[7];
  uint32_t inItvlNew[6];
  char rulesNew[5];
  byte slavesAddrNew[MAX_SLAVES];
  byte slavesNumNew = 0;

  int c, i, n;
  String l;

  siteIdNew[0] = '\0';
  pwdNew[0] = '\0';
  modesNew[0] = '\0';
  rulesNew[0] = '\0';
  for (int i = 0; i < 6; i++) {
    inItvlNew[i] = 0;
  }

  _print("\r\nPaste the configuration:\r\n");
  if (!_consumeWhites()) {
    return false;
  }
  _port->setTimeout(300);
  while (true) {
    l = _port->readStringUntil(':');
    if (l.endsWith("address")) {
      addressNew = _port->parseInt();
    } else if (l.endsWith("frequency")) {
      frequencyNew = _port->parseInt();
    } else if (l.endsWith("power")) {
      txPowerNew = _port->parseInt();
    } else if (l.endsWith("factor")) {
      sfNew = _port->parseInt();
    } else if (l.endsWith("cycle")) {
      dcNew = _port->parseFloat() * 10;
    } else if (l.endsWith("window")) {
      dcWinNew = _port->parseInt();
    } else if (l.endsWith("ID")) {
      if (!_consumeWhites()) {
        return false;
      }
      n = _port->readBytes(siteIdNew, 3);
      if (n != 3) {
        return false;
      }
      siteIdNew[3] = '\0';
    } else if (l.endsWith("Password")) {
      if (!_consumeWhites()) {
        return false;
      }
      n = _port->readBytes(pwdNew, 16);
      if (n != 16) {
        return false;
      }
      pwdNew[16] = '\0';
    } else if (l.endsWith("modes")) {
      if (!_consumeWhites()) {
        return false;
      }
      n = _port->readBytes(modesNew, 6);
      if (n != 6) {
        return false;
      }
      modesNew[6] = '\0';
    } else if (l.endsWith("rules")) {
      if (!_consumeWhites()) {
        return false;
      }
      n = _port->readBytes(rulesNew, 4);
      if (n != 4) {
        return false;
      }
      rulesNew[4] = '\0';
    } else if (l.endsWith("speed")) {
      speedNew = 0;
      n = _port->parseInt();
      for (i = 1; i < 9; i++) {
        if (SPEEDS[i] == n) {
          speedNew = i;
          break;
        }
      }
      if (speedNew == 0) {
        return false;
      }
    } else if (l.endsWith("parity")) {
      if (!_consumeWhites()) {
        return false;
      }
      n = _port->readBytes(_inBuffer, 1);
      if (n != 1) {
        return false;
      }
      if (_inBuffer[0] == 'E') {
        parityNew = 1;
      } else if (_inBuffer[0] == 'O') {
        parityNew = 2;
      } else if (_inBuffer[0] == 'N') {
        parityNew = 3;
      } else {
        return false;
      }
    } else if (l.endsWith("units")) {
      do {
        n = _port->parseInt();
        if (n > 0) {
          slavesAddrNew[slavesNumNew++] = n;
        } else {
          break;
        }
      } while (slavesNumNew < MAX_SLAVES);
    } else if (l.endsWith("interval")) {
      if (l.indexOf("Input 1") >= 0) {
        inItvlNew[0] = _port->parseInt();
      } else if (l.indexOf("Input 2") >= 0) {
        inItvlNew[1] = _port->parseInt();
      } else if (l.indexOf("Input 3") >= 0) {
        inItvlNew[2] = _port->parseInt();
      } else if (l.indexOf("Input 4") >= 0) {
        inItvlNew[3] = _port->parseInt();
      } else if (l.indexOf("Input 5") >= 0) {
        inItvlNew[4] = _port->parseInt();
      } else if (l.indexOf("Input 6") >= 0) {
        inItvlNew[5] = _port->parseInt();
      } else {
        return false;
      }
    } else {
      break;
    }
  }

  if (addressNew == 0 || dcNew == 0 || dcWinNew == 0 || siteIdNew[0] == '\0' ||
      pwdNew[0] == '\0' || modesNew[0] == '\0' || rulesNew[0] == '\0') {
    return false;
  }

  _confirmConfiguration(addressNew, speedNew, parityNew,
    frequencyNew, txPowerNew, sfNew, dcNew, dcWinNew,
    siteIdNew, pwdNew, modesNew,
    inItvlNew[0], inItvlNew[1], inItvlNew[2], inItvlNew[3], inItvlNew[4], inItvlNew[5],
    rulesNew, slavesAddrNew, slavesNumNew);
}

bool SerialConfig::_consumeWhites() {
  int c;
  while (true) {
    c = _port->peek();
    if (c >= 0) {
      if (c == '\b' || c == 127 || c == 27) {
        return false;
      }
      if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
        break;
      }
      _port->read();
    }
  }
  return true;
}

void SerialConfig::_exportConfig() {
  if (!isConfigured) {
    _print("\r\nNot configured\r\n\r\n");
    return;
  }
  _print("\r\n");
  _printConfiguration(address, speed, parity,
    frequency, txPower, sf, dc, dcWin,
    siteId, pwd, modes,
    inItvl[0], inItvl[1], inItvl[2], inItvl[3], inItvl[4], inItvl[5],
    rules, slavesAddr, slavesNum);
  _print("\r\n");
}

void SerialConfig::_enterConfigWizard() {
  byte addressNew;
  byte speedNew;
  byte parityNew;
  uint32_t frequencyNew;
  byte txPowerNew;
  byte sfNew;
  uint16_t dcNew;
  uint16_t dcWinNew;
  byte siteIdNew[4];
  byte pwdNew[17];
  char modesNew[7];
  uint32_t inItvlNew[6];
  char rulesNew[5];
  byte slavesAddrNew[MAX_SLAVES];
  byte slavesNumNew;

  _print("\r\nSelect mode:\r\n"
         "[Press enter to leave current setting: ");
  if (isGateway) {
    _print("1");
  } else {
    _print("2");
  }
  _print("]\r\n"
         "\r\n    1. Gateway"
         "\r\n    2. Remote unit"
         "\r\n\r\n> ");
  _readEchoLine(1, false, false, &_betweenFilter, '1', '2');
  if (_inBuffer[0] != '\0') {
    isGateway = _inBuffer[0] == '1';
  }

  _print("\r\nEnter Modbus address (1-247):\r\n"
         "[Press enter to leave current setting: ");
  _print(address);
  _print("]\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(3, false, false, &_betweenFilter, '0', '9');
    if (_inBuffer[0] != '\0') {
      addressNew = atoi(_inBuffer);
    } else {
      addressNew = address;
    }
  } while (addressNew < 1 || addressNew > 247);

  _print("\r\nEnter LoRa frequency [KHz] (EU: 863000-870000, US: 902000-928000):\r\n"
         "[Press enter to leave current setting: ");
  _print(frequency);
  _print("]\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(6, false, false, &_betweenFilter, '0', '9');
    if (_inBuffer[0] != '\0') {
      frequencyNew = atol(_inBuffer);
      if (frequencyNew >= 868700l && frequencyNew <= 869399l) {
        dc = 1;
      } else if (frequencyNew >= 869400l && frequencyNew <= 869659l) {
        dc = 100;
      } else {
        dc = 10;
      }
    } else {
      frequencyNew = frequency;
    }
  } while (frequencyNew < 400000l);

  _print("\r\nEnter LoRa TX power (2-20):\r\n"
         "[Press enter to leave current setting: ");
  _print(txPower);
  _print("]\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(2, false, false, &_betweenFilter, '0', '9');
    if (_inBuffer[0] != '\0') {
      txPowerNew = atoi(_inBuffer);
    } else {
      txPowerNew = txPower;
    }
  } while (txPowerNew < 2 || txPowerNew > 20);

  _print("\r\nEnter LoRa spreading factor (7-12):\r\n"
         "[Press enter to leave current setting: ");
  _print(sf);
  _print("]\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(2, false, false, &_betweenFilter, '0', '9');
    if (_inBuffer[0] != '\0') {
      sfNew = atoi(_inBuffer);
    } else {
      sfNew = sf;
    }
  } while (sfNew < 7 || sfNew > 12);

  _print("\r\nEnter duty cyle percentage (0.1-100.0):\r\n"
         "[Press enter to leave current setting: ");
  _print(dc/10.0);
  _print("]\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(5, false, false, &_betweenFilter, '.', '9');
    if (_inBuffer[0] != '\0') {
      dcNew = atof(_inBuffer) * 10;
    } else {
      dcNew = dc;
    }
  } while (dcNew < 1 || dcNew > 1000);

  _print("\r\nEnter duty cyle window [seconds] (10-3600):\r\n"
         "[Press enter to leave current setting: ");
  _print(dcWin);
  _print("]\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(4, false, false, &_betweenFilter, '0', '9');
    if (_inBuffer[0] != '\0') {
      dcWinNew = atoi(_inBuffer);
    } else {
      dcWinNew = dcWin;
    }
  } while (dcWinNew < 10 || dcWinNew > 3600);

  if (siteId[0] == '\0') {
    randomSeed(millis());
    int i;
    for (i = 0; i < 3; i++) {
      siteId[i] = random('!', '~');
    }
    siteId[3] = '\0';
    for (i = 0; i < 16; i++) {
      pwd[i] = random('!', '~');
    }
    pwd[16] = '\0';
  }

  _print("\r\nEnter site ID (3 ASCII chars):\r\n"
         "[Press enter to leave current setting: ");
  _print((char *) siteId);
  _print("]\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(3, false, false, &_betweenFilter, '!', '~');
    if (_inBuffer[0] != '\0') {
      strcpy((char *) siteIdNew, _inBuffer);
    } else {
      strcpy((char *) siteIdNew, (char *) siteId);
    }
  } while (strlen((char *) siteIdNew) < 3);

  _print("\r\nEnter password (16 ASCII chars):\r\n"
         "[Press enter to leave current setting: ");
  _print((char *) pwd);
  _print("]\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(16, false, false, &_betweenFilter, '!', '~');
    if (_inBuffer[0] != '\0') {
      strcpy((char *) pwdNew, _inBuffer);
    } else {
      strcpy((char *) pwdNew, (char *) pwd);
    }
  } while (strlen((char *) pwdNew) < 16);

  _print("\r\nEnter input modes [XXXXXX] (D: digital (DIx), V: voltage (AVx), I: current (AIx), -: ignore):\r\n"
         "[Press enter to leave current setting: ");
  _print(modes);
  _print("]\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(6, false, true, &_modesFilter, 0, 0);
    if (_inBuffer[0] != '\0') {
      strcpy(modesNew, _inBuffer);
    } else {
      strcpy(modesNew, modes);
    }
  } while (strlen(modesNew) < 6);

  _print("\r\nEnter I/O rules [XXXX] (F: follow, I: invert, H: flip on L>H transition, L: flip on H>L transition, T: flip on any transition, -: no rule):\r\n"
         "[Press enter to leave current setting: ");
  _print(rules);
  _print("]\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(4, false, true, &_rulesFilter, 0, 0);
    if (_inBuffer[0] != '\0') {
      strcpy(rulesNew, _inBuffer);
    } else {
      strcpy(rulesNew, rules);
    }
  } while (strlen(rulesNew) < 4);

  if (isGateway) {
    _print("\r\nSelect serial port speed:\r\n"
           "[Press enter to leave current setting: ");
    _print(speed);
    _print("]\r\n"
           "\r\n    1. 1200"
           "\r\n    2. 2400"
           "\r\n    3. 4800"
           "\r\n    4. 9600"
           "\r\n    5. 19200"
           "\r\n    6. 38400"
           "\r\n    7. 57600"
           "\r\n    8. 115200"
           "\r\n\r\n");
    do {
      _print("> ");
      _readEchoLine(1, false, false, &_betweenFilter, '1', '8');
      if (_inBuffer[0] != '\0') {
        speedNew = atoi(_inBuffer);
      } else {
        speedNew = speed;
      }
    } while (speedNew < 1 || speedNew > 8);

    _print("\r\nSelect serial port parity:\r\n"
           "[Press enter to leave current setting: ");
    _print(parity);
    _print("]\r\n"
           "\r\n    1. Even"
           "\r\n    2. Odd"
           "\r\n    3. None"
           "\r\n\r\n");
    do {
      _print("> ");
      _readEchoLine(1, false, false, &_betweenFilter, '1', '3');
      if (_inBuffer[0] != '\0') {
        parityNew = atoi(_inBuffer);
      } else {
        parityNew = parity;
      }
    } while (parityNew < 1 || parityNew > 3);

    _print("\r\nEnter '0' for auto-discovery or the address of each remote unit followed by '0' when done:\r\n"
           "[Press enter to leave current setting: ");
    if (slavesNum > 0) {
     for (int i = 0; i < slavesNum; i++) {
       if (i != 0) {
         _print(", ");
       }
       _print(slavesAddr[i]);
     }
    } else {
     _print("auto-discovery");
    }
    _print("]\r\n\r\n");
    slavesNumNew = 0;
    int slAddr;
    do {
      _print("> ");
      _readEchoLine(3, false, false, &_betweenFilter, '0', '9');
      if (_inBuffer[0] != '\0') {
        slAddr = atoi(_inBuffer);
        if (slAddr < 0 || slAddr > 247 || slAddr == addressNew) {
          continue;
        }
        int i = 0;
        for (; i < slavesNumNew; i++) {
          if (slavesAddrNew[i] == slAddr) {
            // duplicate address
            break;
          }
        }
        if (i == slavesNumNew) {
          // not a duplicate
          if (slAddr == 0) {
            break;
          } else {
            slavesAddrNew[slavesNumNew++] = slAddr;
            if (slavesNumNew >= MAX_SLAVES) {
              break;
            }
          }
        }
      } else {
        memcpy(slavesAddrNew, slavesAddr, sizeof(byte) * slavesNum);
        slavesNumNew = slavesNum;
        break;
      }
    } while (true);

    for (int i = 0; i < 6; i++) {
      inItvlNew[i] = 0;
    }

  } else { // remote unit
    speedNew = 0;
    parityNew = 0;
    slavesNumNew = 0;

    bool hasIns = false;
    for (int i = 0; i < 6; i++) {
      if (modesNew[i] != '-') {
        hasIns = true;
        break;
      }
    }

    if (hasIns) {
      _print("\r\nEnter the inputs' updates interval [seconds] (0-65535):\r\n"
             "[Press enter to leave current setting]\r\n\r\n");
      for (int i = 0; i < 6; i++) {
        if (modesNew[i] != '-') {
          do {
            _print("Input ");
            _print(i + 1);
            _print(" [current: ");
            _print(inItvl[i]);
            _print("]:\r\n");
            _print("> ");
            _readEchoLine(5, false, false, &_betweenFilter, '0', '9');
            if (_inBuffer[0] != '\0') {
              inItvlNew[i] = atoi(_inBuffer);
            } else {
              inItvlNew[i] = inItvl[i];
            }
          } while (inItvlNew[i] < 0 || inItvlNew[i] > 65535);
        }
      }
    }
  }

  _confirmConfiguration(addressNew, speedNew, parityNew,
    frequencyNew, txPowerNew, sfNew, dcNew, dcWinNew,
    siteIdNew, pwdNew, modesNew,
    inItvlNew[0], inItvlNew[1], inItvlNew[2], inItvlNew[3], inItvlNew[4], inItvlNew[5],
    rulesNew, slavesAddrNew, slavesNumNew);
}

template <typename T>
void SerialConfig::_print(T text) {
  digitalWrite(PIN_TXEN, HIGH);
  _port->print(text);
  _port->flush();
  delay(5);
  digitalWrite(PIN_TXEN, LOW);
}

int SerialConfig::_orFilter(int c, int idx, int p1, int p2) {
  if (c == p1 || c == p2) {
    return c;
  }
  return -1;
}

int SerialConfig::_betweenFilter(int c, int idx, int min, int max) {
  if (c >= min && c <= max) {
    return c;
  }
  return -1;
}

int SerialConfig::_modesFilter(int c, int idx, int p1, int p2) {
  if (c == 'D' || c == '-' || (idx < 4 && (c == 'V' || c == 'I'))) {
    return c;
  }
  return -1;
}

int SerialConfig::_rulesFilter(int c, int idx, int p1, int p2) {
  if (c == 'F' || c == 'I' || c == 'H' || c == 'L' || c == 'T' || c == '-') {
    return c;
  }
  return -1;
}

void SerialConfig::_readEchoLine(int maxLen, bool returnOnMaxLen,
      bool upperCase, int (*charFilter)(int, int, int, int), int p1, int p2) {
  int c, i = 0, p = 0;
  bool eol = false;
  while (true) {
    while (_port->available() && !eol) {
      c = _port->read();
      switch (c) {
        case '\r':
        case '\n':
          eol = true;
          break;
        case '\b':
        case 127:
          if (i > 0) {
            i--;
          }
          break;
        default:
          if (i < maxLen) {
            if (upperCase && c >= 'a') {
              c -= 32;
            }
            c = charFilter(c, i, p1, p2);
            if (c >= 0) {
              _inBuffer[i++] = c;
            }
          }
          if (returnOnMaxLen && i >= maxLen) {
            eol = true;
          }
          break;
      }
      delay(5);
    }

    for (; p < i; p++) {
      _print(_inBuffer[p]);
    }
    for (; p > i; p--) {
      _print("\b \b");
    }
    if (eol) {
      _inBuffer[i] = '\0';
      _print("\r\n");
      return;
    }
  }
}

bool SerialConfig::_writeEepromConfig(byte address, byte speed, byte parity,
    uint32_t frequency, byte txPower, byte sf, uint16_t dc, uint16_t dcWin,
    byte *siteId, byte *pwd, char *modes,
    uint32_t inItvl1, uint32_t inItvl2, uint32_t inItvl3,
    uint32_t inItvl4, uint32_t inItvl5, uint32_t inItvl6,
    char *rules, byte *slavesAddr, byte slavesNum) {
  byte fb;
  byte checksum = 7;

  EEPROM.write(0, address);
  checksum ^= address;
  EEPROM.write(1, speed);
  checksum ^= speed;
  EEPROM.write(2, parity);
  checksum ^= parity;
  for (int a = 0; a < 4; a++) {
    fb = (byte) ((frequency >> (8 * a)) & 0xff);
    EEPROM.write(a + 3, fb);
    checksum ^= fb;
  }
  EEPROM.write(7, txPower);
  checksum ^= txPower;
  EEPROM.write(8, sf);
  checksum ^= sf;
  for (int a = 0; a < 2; a++) {
    fb = (byte) ((dc >> (8 * a)) & 0xff);
    EEPROM.write(a + 9, fb);
    checksum ^= fb;
  }
  for (int a = 0; a < 2; a++) {
    fb = (byte) ((dcWin >> (8 * a)) & 0xff);
    EEPROM.write(a + 11, fb);
    checksum ^= fb;
  }
  for (int a = 0; a < 3; a++) {
    EEPROM.write(a + 13, siteId[a]);
    checksum ^= siteId[a];
  }
  for (int a = 0; a < 16; a++) {
    EEPROM.write(a + 16, pwd[a]);
    checksum ^= pwd[a];
  }
  for (int a = 0; a < 6; a++) {
    EEPROM.write(a + 32, modes[a]);
    checksum ^= modes[a];
  }
  for (int a = 0; a < 2; a++) {
    fb = (byte) ((inItvl1 >> (8 * a)) & 0xff);
    EEPROM.write(a + 38, fb);
    checksum ^= fb;
  }
  for (int a = 0; a < 2; a++) {
    fb = (byte) ((inItvl2 >> (8 * a)) & 0xff);
    EEPROM.write(a + 40, fb);
    checksum ^= fb;
  }
  for (int a = 0; a < 2; a++) {
    fb = (byte) ((inItvl3 >> (8 * a)) & 0xff);
    EEPROM.write(a + 42, fb);
    checksum ^= fb;
  }
  for (int a = 0; a < 2; a++) {
    fb = (byte) ((inItvl4 >> (8 * a)) & 0xff);
    EEPROM.write(a + 44, fb);
    checksum ^= fb;
  }
  for (int a = 0; a < 2; a++) {
    fb = (byte) ((inItvl5 >> (8 * a)) & 0xff);
    EEPROM.write(a + 46, fb);
    checksum ^= fb;
  }
  for (int a = 0; a < 2; a++) {
    fb = (byte) ((inItvl6 >> (8 * a)) & 0xff);
    EEPROM.write(a + 48, fb);
    checksum ^= fb;
  }
  for (int a = 0; a < 4; a++) {
    EEPROM.write(a + 50, rules[a]);
    checksum ^= rules[a];
  }
  EEPROM.write(54, slavesNum);
  checksum ^= slavesNum;

  EEPROM.write(55, checksum);

  for (int a = 0; a < slavesNum; a++) {
    EEPROM.write(a + 56, slavesAddr[a]);
    checksum ^= slavesAddr[a];
  }

  EEPROM.commit();

  return true;
}

bool SerialConfig::_readEepromConfig() {
  if (!EEPROM.isValid()) {
    return false;
  }

  byte checksum = 7;
  byte mem[55];
  for (int a = 0; a < 55; a++) {
    mem[a] = EEPROM.read(a);
    checksum ^= mem[a];
  }
  if ((EEPROM.read(55) != checksum)) {
    return false;
  }

  address = mem[0];
  speed = mem[1];
  parity = mem[2];
  frequency = (mem[3] & 0xffl)
            + ((mem[4] & 0xffl) << 8)
            + ((mem[5] & 0xffl) << 16)
            + ((mem[6] & 0xffl) << 24);
  txPower = mem[7];
  sf = mem[8];
  dc = (mem[9] & 0xffl) + ((mem[10] & 0xff) << 8);
  dcWin = (mem[11] & 0xffl) + ((mem[12] & 0xff) << 8);
  for (int a = 0; a < 3; a++) {
    siteId[a] = mem[a + 13];
  }
  for (int a = 0; a < 16; a++) {
    pwd[a] = mem[a + 16];
  }
  for (int a = 0; a < 6; a++) {
    modes[a] = mem[a + 32];
  }
  inItvl[0] = (mem[38] & 0xffl) + ((mem[39] & 0xff) << 8);
  inItvl[1] = (mem[40] & 0xffl) + ((mem[41] & 0xff) << 8);
  inItvl[2] = (mem[42] & 0xffl) + ((mem[43] & 0xff) << 8);
  inItvl[3] = (mem[44] & 0xffl) + ((mem[45] & 0xff) << 8);
  inItvl[4] = (mem[46] & 0xffl) + ((mem[47] & 0xff) << 8);
  inItvl[5] = (mem[48] & 0xffl) + ((mem[49] & 0xff) << 8);
  for (int a = 0; a < 4; a++) {
    rules[a] = mem[a + 50];
  }
  slavesNum = mem[54];
  for (int i = 0; i < slavesNum; i++) {
    slavesAddr[i] = EEPROM.read(56 + i);
  }

  return true;
}

void SerialConfig::_confirmConfiguration(byte address, byte speed, byte parity,
    uint32_t frequency, byte txPower, byte sf, uint16_t dc, uint16_t dcWin,
    byte *siteId, byte *pwd, char *modes,
    uint32_t inItvl1, uint32_t inItvl2, uint32_t inItvl3,
    uint32_t inItvl4, uint32_t inItvl5, uint32_t inItvl6,
    char *rules, byte *slavesAddr, byte slavesNum) {

  _print("\r\nNew configuration:\r\n");

  _printConfiguration(address, speed, parity,
    frequency, txPower, sf, dc, dcWin,
    siteId, pwd, modes,
    inItvl1, inItvl2, inItvl3, inItvl4, inItvl5, inItvl6,
    rules, slavesAddr, slavesNum);

  _print("\r\nConfirm? (Y/N):\r\n\r\n");
  do {
    _print("> ");
    _readEchoLine(1, false, true, &_orFilter, 'Y', 'N');
    if (_inBuffer[0] == 'Y') {
      _print("\r\nSaving...");
      _writeEepromConfig(address, speed, parity,
        frequency, txPower, sf, dc, dcWin,
        siteId, pwd, modes,
        inItvl1, inItvl2, inItvl3, inItvl4, inItvl5, inItvl6,
        rules, slavesAddr, slavesNum);
      if (_readEepromConfig()) {
        _print("\r\nSaved!\r\nResetting... bye!\r\n\r\n");
        delay(1000);
        NVIC_SystemReset();
      } else {
        _print("\r\nError\r\n\r\n");
      }
      break;
    } else if (_inBuffer[0] == 'N') {
      break;
    }
  } while (true);
}

void SerialConfig::_printConfiguration(byte address, byte speed, byte parity,
    uint32_t frequency, byte txPower, byte sf, uint16_t dc, uint16_t dcWin,
    byte *siteId, byte *pwd, char *modes,
    uint32_t inItvl1, uint32_t inItvl2, uint32_t inItvl3,
    uint32_t inItvl4, uint32_t inItvl5, uint32_t inItvl6,
    char *rules, byte *slavesAddr, byte slavesNum) {

  bool isGateway = (speed >= 1 && speed <= 8);

  if (isGateway) {
    _print("\r\n[GATEWAY]\r\n");
  } else {
    _print("\r\n[REMOTE UNIT]\r\n");
  }

  _print("\r\nUnit address: ");
  if (address != 0) {
    _print(address);
  } else {
    _print("XXX");
  }
  _print("\r\nLoRa frequency: ");
  _print(frequency);
  _print("\r\nLoRa TX power: ");
  _print(txPower);
  _print("\r\nLoRa spreading factor: ");
  _print(sf);
  _print("\r\nLoRa duty cycle: ");
  _print(dc/10.0);
  _print("\r\nLoRa duty cycle window: ");
  _print(dcWin);
  _print("\r\nSite ID: ");
  _print((char *) siteId);
  _print("\r\nPassword: ");
  _print((char *) pwd);
  _print("\r\nInput modes: ");
  _print(modes);
  _print("\r\nI/O rules: ");
  _print(rules);

  if (isGateway) {
    _print("\r\nSerial speed: ");
    _print(SPEEDS[speed]);
    _print("\r\nSerial parity: ");
    switch (parity) {
      case 1:
        _print("Even");
        break;
      case 2:
        _print("Odd");
        break;
      case 3:
        _print("None");
        break;
    }
    _print("\r\nRemote units: ");
    if (slavesNum > 0) {
      for (int i = 0; i < slavesNum; i++) {
        if (i != 0) {
          _print(", ");
        }
        _print(slavesAddr[i]);
      }
    } else {
      _print("auto-discovery");
    }
  } else {
    if (modes[0] != '-') {
      _print("\r\nInput 1 updates interval: ");
      _print(inItvl1);
    }
    if (modes[1] != '-') {
      _print("\r\nInput 2 updates interval: ");
      _print(inItvl2);
    }
    if (modes[2] != '-') {
      _print("\r\nInput 3 updates interval: ");
      _print(inItvl3);
    }
    if (modes[3] != '-') {
      _print("\r\nInput 4 updates interval: ");
      _print(inItvl4);
    }
    if (modes[4] != '-') {
      _print("\r\nInput 5 updates interval: ");
      _print(inItvl5);
    }
    if (modes[5] != '-') {
      _print("\r\nInput 6 updates interval: ");
      _print(inItvl6);
    }
  }
  _print("\r\n");
}

extern SerialConfig SerialConfig;

#endif
