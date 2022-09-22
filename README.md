# LoRaBus

LoRaBus brings LoRa-enabled devices on a Modbus RTU network.

As of now, LoRaBus is only supported on [Iono MKR](https://www.sferalabs.cc/product/iono-mkr/), which can be used both as LoRa-Modbus gateway or as LoRa remote unit.

The LoRaBus sketch requires the following libraries:
* [Iono](https://github.com/sfera-labs/iono/tree/master/Iono)
* [IonoModbusRtuSlave](https://github.com/sfera-labs/iono/tree/master/IonoModbusRtu)
  * which depends on [ModbusRtuSlave](https://github.com/sfera-labs/arduino-modbus-rtu-slave)
* [LoRa](https://github.com/sfera-labs/arduino-LoRa)
* [IonoLoRaNet](https://github.com/sfera-labs/iono/tree/master/IonoLoRaNet)
  * which depends on [LoRaNet](https://github.com/sfera-labs/arduino-lora-net)

It has been tested on Iono MKR with Arduino MKR WAN 1300/1310 boards running firmware version 1.2.3.    
To update the firmware, download the [MKRWAN library](https://github.com/arduino-libraries/MKRWAN) (version 1.1.0) and run the "MKRWANFWUpdate_standalone" example.

## Architecture

The LoRaBus network comprises one gateway and several remote nodes.
The gateway is directly connected to the RS-485 bus and communicates via LoRa to the remote nodes to send commands and receive state updates.
Each node responds to a different Modbus address.

For details about the protocol used for LoRa messages between remote units and gateway refer to the [LoRaNet library](https://github.com/sfera-labs/arduino-lora-net).

## Configuration

The nodes can be configured via serial console.
The configuration console can be accessed through the module's RS-485 port or Arduino's USB port using any serial communication application (e.g. the Serial Monitor of the Arduino IDE).

Set the communication speed to 9600, 8 bits, no parity, no flow-control and connect the cable.

When the module is powered-up or reset, you can enter console mode by typing five or more consecutive space characters within 10 seconds from reset. If any other character is received, the module will start running in LoRaBus mode.

```
=== Sfera Labs - LoRaBus configuration - v1.0 ===

    1. Configuration wizard
    2. Import configuration
    3. Export configuration

>
```

By selecting `1` you enter the configuration wizard that walks you step by step in configuring all parameters.

After a unit is configured you can export its configuration (function `2`) to be then imported (function `3`) after a firmware update or on another unit (with the required modifications).

The exported configuration is printed in the console; copy/paste it to your favourite text editor, save it for backup or modify the required parameters and import it on another unit by selecting function `2` and pasting the whole configuration text in the console.

**Gateway unit configuration example:**

```
[GATEWAY]

Unit address: 1
LoRa frequency: 869500
LoRa TX power: 14
LoRa spreading factor: 7
LoRa duty cycle: 10.00
LoRa duty cycle window: 600
Site ID: abc
Password: 16AsciiCharsPwrd
Input modes: DDVI-D
I/O rules: FI--
Serial speed: 19200
Serial parity: Even
Remote units: 2, 3
```

**Remote unit configuration example:**

```
[REMOTE UNIT]

Unit address: 2
LoRa frequency: 869500
LoRa TX power: 14
LoRa spreading factor: 7
LoRa duty cycle: 10.00
LoRa duty cycle window: 600
Site ID: abc
Password: 16AsciiCharsPwrd
Input modes: VDDI--
I/O rules: -LT-
Input 1 updates interval: 5
Input 2 updates interval: 5
Input 3 updates interval: 5
Input 4 updates interval: 5
Input 5 updates interval: 0
Input 6 updates interval: 0
```

### Common parameters

Each node (gateway and remote nodes) must be assigned a different **Unit address**, which corresponds to the Modbus address to be used to poll th eunit.

All units under the same LoRaBus network must have the same **LoRa radio parameters**.

A higher **spreading factor** lets you cover a larger distance between remote nodes and gateway, but entails a longer time-on-air for LoRa messages, which, in turn, means a higher consumption of the duty cycle.

The **duty cycle** is expressed in 1/1000. To set a 5% duty cycle, enter 50; for a 0.1% duty cycle, enter 1.     
When the specified duty cycle is exceeded the module will stop sending LoRa messages until the end of the current duty cycle window.

**NB** Make sure to set a duty cycle no higher than the allowed one for the selected frequency in your region.

The **duty cycle window** lets you set the time period over which the duty cycle is calculated. It can be set from 10 seconds to 1 hour (3600 seconds).    
Set a small window if you want to make sure that a module is never "muted" for long periods. Set a larger window if, for instance, you foresee having many close updates/commands separated by long pauses.

All units under the same LoRaBus network must have the same **Site ID** and **Password**. The site ID (3 ASCII characters) is used for rapidly discard messages from different LoRaBus networks, while the password (16 ASCII characters) is used to encrypt the content of messages.

If more LoRaBus networks are used in the same area, make sure to set different site IDs and, if possible, use different LoRa frequencies.

The **Inputs modes** parameter is a 6 characters string that specifies if and how each input is used.      
Inputs 1 to 4 can be used as digital (`D`), voltage (`V`) or current (`I`); inputs 5 and 6 only as digital.
On a remote unit, if you do not want an input to trigger LoRa updates (i.e. you are not going to read its state from the gateway), set it to "ignore" (`-`).

With the **I/O rules** parameter you can configure each one of the digital inputs to control the corresponding output relay. The rules string consists of 4 characters, where the leftmost character represents the rule for DI1/DO1 and the rightmost character for DI4/DO4.
The possible rules are:

`F`: follow - the relay is closed when input is high and open when low    
`I`: invert - the relay is closed when input is low and open when high    
`H`: flip on L>H transition - the relay is flipped at any input transition from low to high    
`L`: flip on H>L transition - the relay is flipped at any input transition from high to low    
`T`: flip on any transition - the relay is flipped at any input transition, both high to low and low to high    
`-`: no rule - no control rule set for this relay.    

### Gateway parameters

**Serial speed** and **Serial parity** set the configuration of the RS-485 interface for Modbus communication.    
Serial speed allowed values: `1200`, `2400`, `4800`, `9600`, `19200`, `38400`, `57600`, `115200`.    
Serial parity allowed values: `Even`, `Odd`, `None`.

In **Remote units** you can choose to specify the list of addresses of the remote nodes which are going to be used with this gateway, or `auto-discovery`. If you set the addresses, when the gateway starts, it will actively try to connect to the nodes speeding up the pairing process. If you  set auto-discovery, the gateway will have to wait for the nodes to send a message for the pairing to occur.

### Remote units parameters

The **Input N updates interval** parameters let you limit the frequency of state updates.
After an update has been triggered by an input variation, further variations will be ignored for the specified number of seconds.
Set the interval to 0 to trigger updates on each variation.

## Modbus registers

Refer to the following table for the list of available registers and corresponding supported Modbus functions.

For the "Functions" column:    
1 = Read coils    
2 = Read discrete inputs    
3 = Read holding registers    
4 = Read input registers    
5 = Write single coil    
6 = Write single register    
15 = Write multiple coils    
16 = Write multiple registers    

|Address|R/W|Functions|Size (bits)|Data type|Unit|Description|
|------:|:-:|---------|----|---------|----|-----------|
|99|R|4|16|unsigned short|-|Device ID:<br/>`0x21` for Iono MKR gateway<br/>`0x22` for Iono MKR remote unit|
|1|R/W|1,5,15|1|bool|-|Relay DO1|
|2|R/W|1,5,15|1|bool|-|Relay DO2|
|3|R/W|1,5,15|1|bool|-|Relay DO3|
|4|R/W|1,5,15|1|bool|-|Relay DO4|
|101|R|2|1|bool|-|Digital input DI1, with debounce|
|102|R|2|1|bool|-|Digital input DI2, with debounce|
|103|R|2|1|bool|-|Digital input DI3, with debounce|
|104|R|2|1|bool|-|Digital input DI4, with debounce|
|105|R|2|1|bool|-|Digital input DI5, with debounce|
|106|R|2|1|bool|-|Digital input DI6, with debounce|
|111|R|2|1|bool|-|Digital input DI1, no debounce (gateway only)|
|112|R|2|1|bool|-|Digital input DI2, no debounce (gateway only)|
|113|R|2|1|bool|-|Digital input DI3, no debounce (gateway only)|
|114|R|2|1|bool|-|Digital input DI4, no debounce (gateway only)|
|115|R|2|1|bool|-|Digital input DI5, no debounce (gateway only)|
|116|R|2|1|bool|-|Digital input DI6, no debounce (gateway only)|
|1001|R|4|16|unsigned short|-|DI1 counter, increased on every rising edge, after the debounce filter. Range: 0-65535 (rolls back to 0 after 65535)|
|1002|R|4|16|unsigned short|-|DI2 counter, increased on every rising edge, after the debounce filter. Range: 0-65535 (rolls back to 0 after 65535)|
|1003|R|4|16|unsigned short|-|DI3 counter, increased on every rising edge, after the debounce filter. Range: 0-65535 (rolls back to 0 after 65535)|
|1004|R|4|16|unsigned short|-|DI4 counter, increased on every rising edge, after the debounce filter. Range: 0-65535 (rolls back to 0 after 65535)|
|1005|R|4|16|unsigned short|-|DI5 counter, increased on every rising edge, after the debounce filter. Range: 0-65535 (rolls back to 0 after 65535)|
|1006|R|4|16|unsigned short|-|DI6 counter, increased on every rising edge, after the debounce filter. Range: 0-65535 (rolls back to 0 after 65535)|
|201|R|4|16|unsigned short|mV|Analog voltage input AV1: 0-30000, 65535 if not available|
|202|R|4|16|unsigned short|mV|Analog voltage input AV2: 0-30000, 65535 if not available|
|203|R|4|16|unsigned short|mV|Analog voltage input AV3: 0-30000, 65535 if not available|
|204|R|4|16|unsigned short|mV|Analog voltage input AV4: 0-30000, 65535 if not available|
|301|R|4|16|unsigned short|µA|Analog current input AI1: 0-25000, 65535 if not available|
|302|R|4|16|unsigned short|µA|Analog current input AI2: 0-25000, 65535 if not available|
|303|R|4|16|unsigned short|µA|Analog current input AI3: 0-25000, 65535 if not available|
|304|R|4|16|unsigned short|µA|Analog current input AI4: 0-25000, 65535 if not available|
|601|R/W|3,6|16|unsigned short|mV|Analog voltage output AO1: 0-10000, 65535 if not available|
|5001|R|4|16|signed short|-|LoRa RSSI of the last received packet from this unit (remote units only)|
|5002|R|4|16|unsigned short|dB/1000|LoRa SNR of the last received packet from this unit (remote units only)|
|5101|R|4|16|unsigned short|sec|Age of last state update received from this unit. 65535 is returned if no update has been received (remote units only)|
