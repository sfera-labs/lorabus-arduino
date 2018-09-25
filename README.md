# LoRaBus

LoRaBus brings LoRa-enabled devices on a Modbus RTU network.

This repo contains the implementation of LoRaBus for Arduino MKR-based modules.

As of now, LoRaBus is only supported on [Iono MKR](https://www.sferalabs.cc/iono-mkr/), which can be used both as LoRa-Modbus gateway or as LoRa remote unit.

The LoRaBus sketch requires the following libraries:
* [Iono](https://github.com/sfera-labs/iono/tree/master/Iono)
* [IonoModbusRtuSlave](https://github.com/sfera-labs/iono/tree/master/IonoModbusRtu)
  * which depends on [ModbusRtuSlave](https://github.com/sfera-labs/arduino-modbus-rtu-slave)
* [LoRa](https://github.com/sfera-labs/arduino-LoRa)
* [IonoLoRaNet](https://github.com/sfera-labs/iono/tree/master/IonoLoRaNet)
  * which depends on [LoRaNet](https://github.com/sfera-labs/arduino-lora-net)

## Architecture

The LoRaBus network comprises one gateway and several remote nodes.
The gateway is directly connected to the RS-485 bus and communicates via LoRa to the remote nodes to send commands and receive state updates.
Each node responds to a different Modbus address.

## Configuration

The nodes can be configured via serial console.
The configuration console can be accessed through the module's RS-485 port or Arduino's USB port using any serial communication application (e.g. the Serial Monitor of the Arduino IDE).

Set the communication speed to 9600, 8 bits, no parity, no flow-control and connect the cable.

When the module is powered-up or reset, you can enter console mode by typing five or more consecutive space characters within 10 seconds from reset. If any other character is received, the module will enter the LoRaBus mode.

```
=== Sfera Labs - LoRaBus configuration menu - v0.1 ===

   0. Print current configuration
   1. Set Modbus unit address
   2. Enable/disable serial interface and set speed
   3. Set serial port parity
   4. Enable/disable LoRa and set frequency
   5. Set LoRa TX power
   6. Set LoRa spreading factor
   7. Set LoRa duty cycle
   8. Set LoRa duty cycle window
   9. Set LoRa site-ID and password
   A. Set inputs mode
   B. Set inputs LoRa updates interval
   C. Set input/output rules
   D. Set remote units addresses
   E. Save configuration and restart

Select a function:
```

### Common parameters
Each node (gateway and remote nodes) must be assigned a different Modbus unit address (function 1).

Use functions 4, 5 and 6 to set the LoRa radio parameters. All units under the same LoRaBus network must have the same LoRa radio configuration.     
A higher spreading factor lets you cover a larger distance between remote nodes and gateway, but entails a longer time-on-air for LoRa messages, which, in turn, means a higher consumption of the duty cycle.

Use function 7 to set the maximum duty cycle to be used. the value is expressed in 1/1000, so, to set a 5% duty cycle, enter 50; for a 0.1% duty cycle, enter 1.     
When the specified duty cycle is exceeded the module will stop sending LoRa mesages until the end of the current duty cycle window.

**NB** Make sure to set a duty cycle no higher than the allowed one for the selected frequency.

The duty cycle window (function 8) lets you set the time period over which the duty cycle is calculated. It can be set from 10 seconds to 1 hour (3600 seconds).    
Set a small window if you want to make sure that a module is never "muted" for long periods. Set a larger window if, for instance, you foresee having many close updates/commands separated by long pauses.

Function 9 lets you set a site-ID and a password used for the LoRa messages. All units under the same LoRaBus network must have the same site-ID and password. The site-ID is used for rapidly discard messages from difefrent LoRaBus networks, while the password is used to encrypt the content of messages.     

If more LoRaBus networks are used in the same area, make sure to set different site-IDs and, if possible, use different LoRa frequencies.

Use function C to define input/output rules. With these rules you can configure each one of the digital inputs to control the corresponding output relay. The rules string consists of four characters, where the leftmost character represents the rule for DI1/DO1 and the rightmost character for DI4/DO4.
The possible rules are:

`F`: follow - the relay is closed when input is high and open when low    
`I`: invert - the relay is closed when input is low and open when high    
`H`: flip on L>H transition - the relay is flipped at any input transition from low to high    
`L`: flip on H>L transition - the relay is flipped at any input transition from high to low    
`T`: flip on any transition - the relay is flipped at any input transition, both high to low and low to high    
`-`: no rule - no control rule set for this relay.    

### Gateway parameters
A gateway must have its **serial interface** enabled. Use menu functions 2 and 3 to set the speed and parity.

The **inputs mode** (function A) and the **inputs LoRa updates interval** (function B) are not relevant for the gateway.

With function D you can choose to specify the address of the remote nodes which are going to be used with this gateway. If you do, when the gateway starts, it will actively try to connect to the nodes speeding up the pairing process. If you don't (auto-discovery), the gateway will have to wait for the nodes to send a message to it.

When a module is configured as gateway, its current configuration (function 0) will show as follows:

```
Select a function: 0

Current configuration:
   Unit address: 10
   Serial speed: 115200
   Serial parity: Even
   LoRa frequency: 869500000
   LoRa TX power: 14
   LoRa spreading factor: 7
   LoRa duty cycle (1/1000): 100
   LoRa duty cycle window (seconds): 600
   LoRa site-ID and password: ABC16Chars$ecretK&y
   Input modes: DDDDDD
   I/O rules: FI--
   Remote units: auto-discovery
```

### Remote units parameters
To configure a module as remote unit, disable the **serial interface** from function 2.

The **inputs mode** (function A) must be set to specify if and how a specific input is used.      
Inputs 1 to 4 can be used as digital (`D`), voltage (`V`) or current (`I`); inputs 5 and 6 only as digital.
If you do not want an input to trigger LoRa updates (i.e. you are not going to read its state from the gateway), set it to "ignore" (`-`).

The function **Set inputs LoRa updates interval** (B) lets you limit the frequency of state updates.
After an update has been triggered by an input variation, further variations will be ignored for the specified number of seconds.
Set the interval to 0 to trigger updates on each variation.

When a module is configured as remote unit, its current configuration (function 0) will show as follows:

```
Select a function: 0

Current configuration:
   Unit address: 11
   Serial: disabled
   LoRa frequency: 869500000
   LoRa TX power: 14
   LoRa spreading factor: 7
   LoRa duty cycle (1/1000): 100
   LoRa duty cycle window (seconds): 600
   LoRa site-ID and password: ABC16Chars$ecretK&y
   Input modes: DVVI-D
   Input LoRa updates interval (seconds):
      Input 1: 10
      Input 2: 10
      Input 3: 60
      Input 4: 0
      Input 5: 0
      Input 6: 0
   I/O rules: ----
```

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

|Address|R/W|Functions|Size|Data type|Unit|Description|
|------:|:-:|---------|----|---------|----|-----------|
|99|R|4|1 word|unsigned short|-|Device ID:<br/>`0x21` for Iono MKR gateway<br/>`0x22` for Iono MKR remote unit|

**TBD**
