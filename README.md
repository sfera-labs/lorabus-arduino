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

## Usage

### Configuration

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

#### Common parameters

#### Gateway
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
   I/O rules: ----
   Remote units: auto-discovery
```

#### Remote units
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


