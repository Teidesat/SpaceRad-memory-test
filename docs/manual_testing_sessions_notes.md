# Notes on the manual testing of the code.

## Soldering session

The EEPROM, NOR Flash, MRAM, FRAM memories were soldered during an in person session to the breakout board. The NAND Flash was not soldered due to time constraints, so it cannot be tested yet.

A problem is that the EEPROM might not be in the correct orientation because it didn't have a reference point printed on the chip's shield.

## Physical connections

An Arduino Nano Atmega328p (See [Arduino Reference](https://docs.arduino.cc/hardware/nano), [Datasheet](https://docs.arduino.cc/resources/datasheets/A000005-datasheet.pdf)) was connected to the EEPROM using jumper wires.

The following connections were made:

| Arduino Nano pin name  | Pin number            | EEPROM pin name (breakout board labels)
| ---------------------  | --------------------  | -----------------------
| GND                    | 16                    | GND
| +3V3                   | 1                     | 3V3
| D10                    | 11                    | ¬CS
| D13                    | 14                    | SCK
| D9                     | 10                    | ¬WP
| D8                     | 9                     | ¬HOLD
| D11                    | 12                    | INPUT
| D12                    | 13                    | OUTPUT


