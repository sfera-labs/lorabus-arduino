/*
  SerialConfig.h

    Copyright (C) 2018 Sfera Labs S.r.l. - All rights reserved.

    For information, see the iono web site:
    http://www.sferalabs.cc/

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef SerialConfig_h
#define SerialConfig_h

#ifdef ARDUINO_ARCH_SAMD
#include <FlashAsEEPROM.h>
#include <FlashStorage.h>
#else
#include <EEPROM.h>
#endif

#define MAX_SLAVES  20

#define CONSOLE_TIMEOUT 10000

#define MENU_ITEMS_NUM 15

#ifndef SERIAL_PORT_MONITOR
#define SERIAL_PORT_MONITOR SERIAL_PORT_HARDWARE
#endif

const PROGMEM char *CONSOLE_MENU_HEADER  = "=== Sfera Labs - LoRaBus configuration menu - v0.1 ===";
const PROGMEM char *CONSOLE_MENU_ITEMS[] = {
  "Print current configuration",
  "Set Modbus unit address",
  "Enable/disable serial interface and set speed",
  "Set serial port parity",
  "Enable/disable LoRa and set frequency",
  "Set LoRa TX power",
  "Set LoRa spreading factor",
  "Set LoRa duty cycle",
  "Set LoRa duty cycle window",
  "Set LoRa site-ID and password",
  "Set inputs mode",
  "Set inputs LoRa updates interval",
  "Set input/output rules",
  "Set remote units addresses",
  "Save configuration and restart"
};
const PROGMEM char *CONSOLE_MENU_FOOTER = "Select a function: ";

const PROGMEM char *CONSOLE_MENU_ITEMS_DESCR[] = {
  "Current configuration:",
  "Type unit address\r\n[1-247]: ",
  "Select serial port speed for gateway or disable for LoRa units\r\n[1: 1200, 2: 2400, 3: 4800, 4: 9600, 5: 19200, 6: 38400, 7: 57600, 8: 115200, 9: Disabled]: ",
  "Select serial port parity (gateway only)\r\n[1: Even, 2: Odd, 3: None]: ",
  "Type LoRa radio frequency\r\n[1: Disabled, 863000000-870000000: EU band, 902000000-928000000: US band]: ",
  "Type LoRa TX power\r\n[2-20]: ",
  "Type LoRa spreading factor\r\n[7-12]: ",
  "Type LoRa duty cycle (1/1000)\r\n[1-1000]: ",
  "Type LoRa duty cycle window (seconds)\r\n[10-3600]: ",
  "Type site-ID (3 ASCII chars) and password (16 ASCII chars), e.g. \"ABC16Chars$ecretK&y\"\r\n[SSSPPPPPPPPPPPPPPPP - S,P: printable ASCII char]: ",
  "Type input modes, e.g. \"DDVI-D\"\r\n[XXXXXX - D: digital (DIx), V: voltage (AVx), I: current (AIx), -: ignore]: ",
  "Select an input\r\n[1-6]: ",
  "Type input/output rules, e.g. \"FIH-\"\r\n[XXXX - F: follow, I: invert, H: flip on L>H transition, L: flip on H>L transition, T: flip on any transition, -: no rule]: ",
  "Type the address of each remote unit to add, followed by enter; type '0' when done.\r\n[1-247]: ",
  "New configuration:"
};

const PROGMEM char *CONSOLE_CONFIRM = "Confirm? (Y/N): ";
const PROGMEM char *CONSOLE_ERROR = "Error";
const PROGMEM char *CONSOLE_SAVED = "Saved";

const PROGMEM char *IN_SET_INTERVAL = "Type the interval (in seconds)\r\n[0-65535]: ";

const long SPEEDS[] = {0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

class SerialConfig {
  private:
    static bool _done;
    static unsigned long _bootTimeMillis;
    static bool _validConfiguration;
    static Stream *_port;
    static bool _consoleActive;
    static short _spacesCounter;
    static char _inBuffer[24];
    static byte _menuSelect;
    static byte _inputSelect;

    static byte _addressNew;
    static byte _speedNew;
    static byte _parityNew;
    static uint32_t _frequencyNew;
    static byte _txPowerNew;
    static byte _sfNew;
    static uint16_t _dcNew;
    static uint16_t _dcWinNew;
    static byte _idPwdNew[20];
    static char _modesNew[7];
    static uint32_t _inItvlNew[6];
    static char _rulesNew[5];
    static byte _slavesAddrNew[MAX_SLAVES];
    static byte _slavesNumNew;

    static void _close();
    static void _processChar(int b);
    static void _strcat_c(char *s, char c);
    template <typename T>
    static bool _numberEdit(T *value, int c, int length, long min, long max);
    static bool _modesEdit(char *value, int c, int size);
    static bool _rulesEdit(char *value, int c, int size);
    static bool _idPwdEdit(byte *value, int c, int size);
    static void _printConfiguration(byte address, byte speed, byte parity,
        uint32_t frequency, byte txPower, byte sf, uint16_t dc, uint16_t dcWin,
        byte *idPwd, char *modes,
        uint32_t inItvl1, uint32_t inItvl2, uint32_t inItvl3,
        uint32_t inItvl4, uint32_t inItvl5, uint32_t inItvl6,
        char *rules, byte *slavesAddr, byte slavesNum);
    static void _printMenu();
    static void _printProgMemString(const char* s);
    static void _printlnProgMemString(const char* s);
    static void _softReset();
    static bool _getEEPROMConfig();
    static bool _readEepromConfig();
    static bool _writeEepromConfig(byte address, byte speed, byte parity,
        uint32_t frequency, byte txPower, byte sf, uint16_t dc, uint16_t dcWin,
        byte *idPwd, char *modes,
        uint32_t inItvl1, uint32_t inItvl2, uint32_t inItvl3,
        uint32_t inItvl4, uint32_t inItvl5, uint32_t inItvl6,
        char *rules, byte *slavesAddr, byte slavesNum);
    static bool _saveConfig();

  public:
    static byte address;
    static byte speed;
    static byte parity;
    static uint32_t frequency;
    static byte txPower;
    static byte sf;
    static uint16_t dc;
    static uint16_t dcWin;
    static byte idPwd[20];
    static char modes[7];
    static uint32_t inItvl[6];
    static char rules[5];
    static byte slavesAddr[MAX_SLAVES];
    static byte slavesNum;

    static void setup();
    static bool done();
    static void process();
};

bool SerialConfig::_done = false;
unsigned long SerialConfig::_bootTimeMillis;
bool SerialConfig::_validConfiguration = false;
Stream *SerialConfig::_port = NULL;
bool SerialConfig::_consoleActive = false;
short SerialConfig::_spacesCounter = 0;
char SerialConfig::_inBuffer[24];
byte SerialConfig::_menuSelect = 0;
byte SerialConfig::_inputSelect = 0;

byte SerialConfig::_addressNew;
byte SerialConfig::_speedNew;
byte SerialConfig::_parityNew;
uint32_t SerialConfig::_frequencyNew;
byte SerialConfig::_txPowerNew;
byte SerialConfig::_sfNew;
uint16_t SerialConfig::_dcNew;
uint16_t SerialConfig::_dcWinNew;
byte SerialConfig::_idPwdNew[20];
char SerialConfig::_modesNew[7];
uint32_t SerialConfig::_inItvlNew[6];
char SerialConfig::_rulesNew[5];
byte SerialConfig::_slavesAddrNew[MAX_SLAVES];
byte SerialConfig::_slavesNumNew;

byte SerialConfig::address;
byte SerialConfig::speed;
byte SerialConfig::parity;
uint32_t SerialConfig::frequency;
byte SerialConfig::txPower;
byte SerialConfig::sf;
uint16_t SerialConfig::dc;
uint16_t SerialConfig::dcWin;
byte SerialConfig::idPwd[20];
char SerialConfig::modes[7];
uint32_t SerialConfig::inItvl[6];
char SerialConfig::rules[5];
byte SerialConfig::slavesAddr[MAX_SLAVES];
byte SerialConfig::slavesNum;

void SerialConfig::setup() {
  _bootTimeMillis = millis();

  SERIAL_PORT_HARDWARE.begin(9600);
  if ((int) &SERIAL_PORT_HARDWARE != (int) &SERIAL_PORT_MONITOR) {
    SERIAL_PORT_MONITOR.begin(9600);
  }

  _addressNew = 0;
  _speedNew = 0;
  _parityNew = 0;
  _frequencyNew = 0;
  _txPowerNew = 0;
  _sfNew = 0;
  _dcNew = 0;
  _dcWinNew = 0;
  _idPwdNew[0] = 0;
  _modesNew[0] = 0;
  for (int i = 0; i < 6; i++) {
    _inItvlNew[i] = -1;
  }
  _rulesNew[0] = 0;
  _slavesNumNew = 0xff;

  if (!_readEepromConfig()) {
    address = 0;
    speed = 8;
    parity = 1;
    frequency = 869500000l;
    txPower = 14;
    sf = 7;
    dc = 100;
    dcWin = 600;
    strncpy((char *) idPwd, "ABC16Chars$ecretK&y", 19);
    idPwd[19] = 0;
    strncpy(modes, "DDDDDD", 6);
    modes[6] = 0;
    for (int i = 0; i < 6; i++) {
      inItvl[i] = 0;
    }
    strncpy(rules, "----", 4);
    rules[4] = 0;
    slavesNum = 0;
  }

  bool serialEnabled = (speed >= 1 && speed <= 8);
  bool loraEnabled = frequency >= 863000000l;
  _validConfiguration = address != 0 && (serialEnabled || loraEnabled);
}

void SerialConfig::process() {
  if (_port == NULL) {
    if (SERIAL_PORT_HARDWARE.available()) {
      _port = &SERIAL_PORT_HARDWARE;
    } else if (SERIAL_PORT_MONITOR.available()) {
      _port = &SERIAL_PORT_MONITOR;
    }
  } else if (_port->available()) {
    int b = _port->read();
    if (_consoleActive) {
      _processChar(b);
    } else {
      if (b == ' ') {
        if (_spacesCounter >= 4) {
#ifdef IONO_MKR
          digitalWrite(PIN_TXEN, HIGH);
#endif
          _printMenu();
#ifdef IONO_MKR
          _port->flush();
          digitalWrite(PIN_TXEN, LOW);
#endif
          _consoleActive = true;
        } else {
          _spacesCounter++;
        }
      } else if (_validConfiguration) {
        _close();
      } else {
        _port = NULL;
      }
    }
  }

  if (_validConfiguration && !_consoleActive && _bootTimeMillis + CONSOLE_TIMEOUT < millis()) {
    _close();
  }
}

bool SerialConfig::done() {
  return _done;
}

void SerialConfig::_close() {
  SERIAL_PORT_HARDWARE.end();
  SERIAL_PORT_MONITOR.end();
  _done = true;
}

void SerialConfig::_processChar(int b) {
#ifdef IONO_MKR
  digitalWrite(PIN_TXEN, HIGH);
#endif
  switch (_menuSelect) {
    case 0: // waiting for menu selection
      if (b >= 'a') {
        b -= 32;
      }
      _menuSelect = (b >= 'A') ? b - 'A' + 10 : b - '0';

      if (_menuSelect >= 0 && _menuSelect < MENU_ITEMS_NUM) {
        _port->println((char) b);
        _port->println();
        _printProgMemString(CONSOLE_MENU_ITEMS_DESCR[_menuSelect]);
        _inBuffer[0] = 0;

        switch (_menuSelect) {
          case 0: // Print current configuration
            _port->println();
            _printConfiguration(address, speed, parity,
              frequency, txPower, sf, dc, dcWin,
              idPwd, modes,
              inItvl[0], inItvl[1], inItvl[2], inItvl[3], inItvl[4], inItvl[5],
              rules, slavesAddr, slavesNum);
            _printMenu();
            break;
          case 13: // remote units addresses
            _slavesNumNew = 0;
            break;
          case 14: // Save config
            _port->println();
            _printConfiguration(
              (_addressNew == 0) ? address : _addressNew,
              (_speedNew == 0) ? speed : _speedNew,
              (_parityNew == 0) ? parity : _parityNew,
              (_frequencyNew == 0) ? frequency : _frequencyNew,
              (_txPowerNew == 0) ? txPower : _txPowerNew,
              (_sfNew == 0) ? sf : _sfNew,
              (_dcNew == 0) ? dc : _dcNew,
              (_dcWinNew == 0) ? dcWin : _dcWinNew,
              (_idPwdNew[0] == 0) ? idPwd : _idPwdNew,
              (_modesNew[0] == 0) ? modes : _modesNew,
              (_inItvlNew[0] == -1) ? inItvl[0] : _inItvlNew[0],
              (_inItvlNew[1] == -1) ? inItvl[1] : _inItvlNew[1],
              (_inItvlNew[2] == -1) ? inItvl[2] : _inItvlNew[2],
              (_inItvlNew[3] == -1) ? inItvl[3] : _inItvlNew[3],
              (_inItvlNew[4] == -1) ? inItvl[4] : _inItvlNew[4],
              (_inItvlNew[5] == -1) ? inItvl[5] : _inItvlNew[5],
              (_rulesNew[0] == 0) ? rules : _rulesNew,
              (_slavesNumNew == 0xff) ? slavesAddr : _slavesAddrNew,
              (_slavesNumNew == 0xff) ? slavesNum : _slavesNumNew
            );
            _port->println();
            _printProgMemString(CONSOLE_CONFIRM);
            break;
        }
      } else {
        _menuSelect = 0;
      }
      break;
    case 1: // Modbus address
      if (_numberEdit(&_addressNew, b, 3, 1, 247)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 2: // Modbus speed
      if (_numberEdit(&_speedNew, b, 1, 1, 9)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 3: // Modbus parity
      if (_numberEdit(&_parityNew, b, 1, 1, 3)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 4: // LoRa frequency
      if (_numberEdit(&_frequencyNew, b, 9, 1l, 928000000l)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 5: // LoRa TX power
      if (_numberEdit(&_txPowerNew, b, 2, 2, 20)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 6: // LoRa spreading factor
      if (_numberEdit(&_sfNew, b, 2, 7, 12)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 7: // LoRa duty cycle
      if (_numberEdit(&_dcNew, b, 4, 1, 1000)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 8: // LoRa duty cycle window
      if (_numberEdit(&_dcWinNew, b, 4, 10, 3600)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 9: // site-ID and password
      if (_idPwdEdit(_idPwdNew, b, 19)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 10: // Input modes
      if (_modesEdit(_modesNew, b, 6)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 11: // Input update interval: select
      if (b >= '1' && b <= '6') {
        _menuSelect = 111;
        _inputSelect = b - '0';
        _port->println(_inputSelect);
        _printProgMemString(IN_SET_INTERVAL);
      }
      break;
    case 111: // Input update interval: enter val
      if (_numberEdit(&_inItvlNew[_inputSelect - 1], b, 5, 0, 65535)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 12: // IO rules
      if (_rulesEdit(_rulesNew, b, 4)) {
        _menuSelect = 0;
        _port->println();
        _printMenu();
      }
      break;
    case 13: // remote units addresses
      if (_numberEdit(&_slavesAddrNew[_slavesNumNew], b, 3, 0, 247)) {
        int i = 0;
        for (; i < _slavesNumNew; i++) {
          if (_slavesAddrNew[i] == _slavesAddrNew[_slavesNumNew]) {
            // duplicate address
            break;
          }
        }
        if (i == _slavesNumNew) {
          // not a duplicate
          bool exit = false;
          if (_slavesAddrNew[_slavesNumNew] == 0) {
            exit = true;
          } else {
            _slavesNumNew++;
            if (_slavesNumNew >= MAX_SLAVES) {
              exit = true;
            }
          }
          if (exit) {
            _menuSelect = 0;
            _port->println();
            _printMenu();
            break;
          }
        }
        _inBuffer[0] = 0;
        _port->print("\r\n[1-247]: ");
      }
      break;
    case 14: // save
      switch (b) {
        case 'Y':
        case 'y':
          _menuSelect = 0;
          _port->println('Y');
          if (_saveConfig()) {
            _printlnProgMemString(CONSOLE_SAVED);
            delay(1000);
            _softReset();
          } else {
            _printlnProgMemString(CONSOLE_ERROR);
          }
          _printMenu();
          break;
        case 'N':
        case 'n':
          _menuSelect = 0;
          _port->println('N');
          _port->println();
          _printMenu();
          break;
      }
      break;
    default:
      break;
  }
#ifdef IONO_MKR
  _port->flush();
  digitalWrite(PIN_TXEN, LOW);
#endif
}

bool SerialConfig::_saveConfig() {
  return _writeEepromConfig(
    (_addressNew == 0) ? address : _addressNew,
    (_speedNew == 0) ? speed : _speedNew,
    (_parityNew == 0) ? parity : _parityNew,
    (_frequencyNew == 0) ? frequency : _frequencyNew,
    (_txPowerNew == 0) ? txPower : _txPowerNew,
    (_sfNew == 0) ? sf : _sfNew,
    (_dcNew == 0) ? dc : _dcNew,
    (_dcWinNew == 0) ? dcWin : _dcWinNew,
    (_idPwdNew[0] == 0) ? idPwd : _idPwdNew,
    (_modesNew[0] == 0) ? modes : _modesNew,
    (_inItvlNew[0] == -1) ? inItvl[0] : _inItvlNew[0],
    (_inItvlNew[1] == -1) ? inItvl[1] : _inItvlNew[1],
    (_inItvlNew[2] == -1) ? inItvl[2] : _inItvlNew[2],
    (_inItvlNew[3] == -1) ? inItvl[3] : _inItvlNew[3],
    (_inItvlNew[4] == -1) ? inItvl[4] : _inItvlNew[4],
    (_inItvlNew[5] == -1) ? inItvl[5] : _inItvlNew[5],
    (_rulesNew[0] == 0) ? rules : _rulesNew,
    (_slavesNumNew == 0xff) ? slavesAddr : _slavesAddrNew,
    (_slavesNumNew == 0xff) ? slavesNum : _slavesNumNew
  );
}

bool SerialConfig::_writeEepromConfig(byte address, byte speed, byte parity,
    uint32_t frequency, byte txPower, byte sf, uint16_t dc, uint16_t dcWin,
    byte *idPwd, char *modes,
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
  for (int a = 0; a < 19; a++) {
    EEPROM.write(a + 13, idPwd[a]);
    checksum ^= idPwd[a];
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

#ifdef ARDUINO_ARCH_SAMD
  EEPROM.commit();
#endif

  return true;
}

bool SerialConfig::_readEepromConfig() {
#ifdef ARDUINO_ARCH_SAMD
  if (!EEPROM.isValid()) {
    return false;
  }
#endif

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
  for (int a = 0; a < 19; a++) {
    idPwd[a] = mem[a + 13];
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

void SerialConfig::_softReset() {
#ifdef ARDUINO_ARCH_SAMD
  NVIC_SystemReset();
#else
  asm volatile ("  jmp 0");
#endif
}

void SerialConfig::_printlnProgMemString(const char* s) {
  _printProgMemString(s);
  _port->println();
}

void SerialConfig::_printProgMemString(const char* s) {
  int len = strlen_P(s);
  for (int k = 0; k < len; k++) {
    _port->print((char)pgm_read_byte_near(s + k));
  }
}

void SerialConfig::_printMenu() {
  _port->println();
  _printlnProgMemString(CONSOLE_MENU_HEADER);
  _port->println();
  for (int i = 0; i < MENU_ITEMS_NUM; i++) {
    _port->print("   ");
    _port->print(i, HEX);
    _port->print(". ");
    _printlnProgMemString(CONSOLE_MENU_ITEMS[i]);
  }
  _port->println();
  _printProgMemString(CONSOLE_MENU_FOOTER);
}

void SerialConfig::_printConfiguration(byte address, byte speed, byte parity,
    uint32_t frequency, byte txPower, byte sf, uint16_t dc, uint16_t dcWin,
    byte *idPwd, char *modes,
    uint32_t inItvl1, uint32_t inItvl2, uint32_t inItvl3,
    uint32_t inItvl4, uint32_t inItvl5, uint32_t inItvl6,
    char *rules, byte *slavesAddr, byte slavesNum) {

  bool serialEnabled = (speed >= 1 && speed <= 8);
  bool loraEnabled = frequency >= 863000000l;

  _port->print("   Unit address: ");
  if (address != 0) {
    _port->print(address);
  }
  _port->println();

  if (serialEnabled) {
    _port->print("   Serial speed: ");
    _port->print(SPEEDS[speed]);
  } else {
    _port->print("   Serial: disabled");
  }
  _port->println();

  if (serialEnabled) {
    _port->print("   Serial parity: ");
    switch (parity) {
      case 1:
        _port->print("Even");
        break;
      case 2:
        _port->print("Odd");
        break;
      case 3:
        _port->print("None");
        break;
    }
    _port->println();
  }

  if (loraEnabled) {
    _port->print("   LoRa frequency: ");
    _port->print(frequency);
  } else {
    _port->print("   LoRa: disabled");
  }
  _port->println();

  if (loraEnabled) {
    _port->print("   LoRa TX power: ");
    if (txPower != 0) {
      _port->print(txPower);
    }
    _port->println();

    _port->print("   LoRa spreading factor: ");
    if (sf != 0) {
      _port->print(sf);
    }
    _port->println();

    _port->print("   LoRa duty cycle (1/1000): ");
    if (dc != 0) {
      _port->print(dc);
    }
    _port->println();

    _port->print("   LoRa duty cycle window (seconds): ");
    if (dcWin != 0) {
      _port->print(dcWin);
    }
    _port->println();

    _port->print("   LoRa site-ID and password: ");
    _port->print((char *) idPwd);
    _port->println();
  }

  _port->print("   Input modes: ");
  _port->print(modes);
  _port->println();

  if (!serialEnabled) {
    _port->println("   Input LoRa updates interval (seconds):");
    _port->print("      Input 1: ");
    _port->println(inItvl1);
    _port->print("      Input 2: ");
    _port->println(inItvl2);
    _port->print("      Input 3: ");
    _port->println(inItvl3);
    _port->print("      Input 4: ");
    _port->println(inItvl4);
    _port->print("      Input 5: ");
    _port->println(inItvl5);
    _port->print("      Input 6: ");
    _port->println(inItvl6);
  }

  _port->print("   I/O rules: ");
  _port->print(rules);
  _port->println();

  if (serialEnabled && loraEnabled) {
    _port->print("   Remote units: ");
    if (slavesNum > 0) {
      for (int i = 0; i < slavesNum; i++) {
        if (i != 0) {
          _port->print(", ");
        }
        _port->print(slavesAddr[i]);
      }
      _port->println();
    } else {
      _port->println("auto-discovery");
    }
  }
}

template <typename T>
bool SerialConfig::_numberEdit(T *value, int c, int length, long min, long max) {
  int i = strlen(_inBuffer);
  switch (c) {
    case 8: case 127: // backspace
      if (i > 0) {
        _port->print('\b');
        _port->print(' ');
        _port->print('\b');
        _inBuffer[i - 1] = 0;
      } else {
        return true;
      }
      break;
    case 10: // newline
    case 13: // enter
      if (i > 0) {
        long v = strtol(_inBuffer, NULL, 10);
        if (v >= min && v <= max) {
          *value = (T) v;
          _port->println();
          _port->print("OK");
        } else {
          _port->println();
          _port->print("ERROR");
        }
        return true;
      }
      break;
    default:
      if (i < length) {
        if (c >= '0' && c <= '9') {
          _port->print((char) c);
          _port->print((char) 0);
          _strcat_c(_inBuffer, c);
        }
      }
  }
  return false;
}

bool SerialConfig::_idPwdEdit(byte *value, int c, int size) {
  int i = strlen(_inBuffer);
  switch (c) {
    case 8: case 127: // backspace
      if (i > 0) {
        _port->print('\b');
        _port->print(' ');
        _port->print('\b');
        _inBuffer[i - 1] = 0;
      } else {
        return true;
      }
      break;
    case 10: // newline
    case 13: // enter
      if (i > 0) {
        if (i == size) {
          strcpy((char *) value, _inBuffer);
          _port->println();
          _port->print("OK");
        } else {
          _port->println();
          _port->print("ERROR");
        }
        return true;
      }
      break;
    default:
      if (i < size) {
        if (c >= ' ' && c <= '~') {
          _port->print((char) c);
          _port->print((char) 0);
          _strcat_c(_inBuffer, c);
        }
      }
      break;
  }
  return false;
}

bool SerialConfig::_modesEdit(char *value, int c, int size) {
  int i = strlen(_inBuffer);
  switch (c) {
    case 8: case 127: // backspace
      if (i > 0) {
        _port->print('\b');
        _port->print(' ');
        _port->print('\b');
        _inBuffer[i - 1] = 0;
      } else {
        return true;
      }
      break;
    case 10: // newline
    case 13: // enter
      if (i > 0) {
        if (i == size) {
          strcpy(value, _inBuffer);
          _port->println();
          _port->print("OK");
        } else {
          _port->println();
          _port->print("ERROR");
        }
        return true;
      }
      break;
    default:
      if (i < size) {
        if (c >= 'a') {
          c -= 32;
        }
        if (c == 'D' || c == '-' || (i < 4 && (c == 'V' || c == 'I'))) {
          _port->print((char) c);
          _port->print((char) 0);
          _strcat_c(_inBuffer, c);
        }
      }
      break;
  }
  return false;
}

bool SerialConfig::_rulesEdit(char *value, int c, int size) {
  int i = strlen(_inBuffer);
  switch (c) {
    case 8: case 127: // backspace
      if (i > 0) {
        _port->print('\b');
        _port->print(' ');
        _port->print('\b');
        _inBuffer[i - 1] = 0;
      }
      break;
    case 10: // newline
    case 13: // enter
      if (i > 0) {
        if (i == size) {
          strcpy(value, _inBuffer);
          _port->println();
          _port->print("OK");
        } else {
          _port->println();
          _port->print("ERROR");
        }
        return true;
      }
      break;
    default:
      if (i < size) {
        if (c >= 'a') {
          c -= 32;
        }
        if (c == 'F' || c == 'I' || c == 'H' || c == 'L' || c == 'T' || c == '-') {
          _port->print((char) c);
          _port->print((char) 0);
          _strcat_c(_inBuffer, c);
        }
      }
      break;
  }
  return false;
}

void SerialConfig::_strcat_c(char *s, char c) {
  for (; *s; s++);
  *s++ = c;
  *s++ = 0;
}

extern SerialConfig SerialConfig;

#endif
