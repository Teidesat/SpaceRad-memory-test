/**
 * @file memory_eeprom.h
 * @author Marcos Barrios
 * @brief Meant for a EEPROM SPI. Model: M95M02-DWMN3TP/K
 * @version 0.1
 * @date 2023-08-24
 * 
 * @copyright Copyright (c) 2023
 * 
 * The following information comes from the datasheet:
 * https://www.mouser.es/datasheet/2/389/m95m02_a125-1849907.pdf
 * from the webpage:
 * https://www.mouser.es/ProductDetail/STMicroelectronics/M95M02-DWMN3TP-K?qs=Ok1pvOkw6%2Fr65R3s1i3vIw%3D%3D
 * 
 * 1.- Because it is SPI based, the pins are the following:
 *    Supply voltage,
 *    Ground,
 *    Serial Clock,
 *    not(Chip select),
 *    not(HOLD),
 *    not(WriteProtect),
 *    Serial Input,
 *    Serial Output
 *
 * 2.- Clock polarity and clock phase required for the SPI communication:
 *  CPOL=0, CPHA=0 or
 *  CPOL=0, CPHA=1
 * Make sure to configure SPI on either. Clock stays on 0 or 1 when
 * master (esp32 probably) is in stand-by-mode and not transfering data.
 *
 * 3.- Memory operations are done by commands:
 *  - WRITE, WRSR, WRID, LID...
 *  - READ, RDSR, RDID, RDLS...
 *  etc...
 *
 * How to write and read:
 *
 * Chip select must be 1 before a write command. It has to be first
 * set to 0, and then a WREN (Write Enable) command must be first executed.
 * After each byte data boundary input, the Chip select must be put back to 1.
 * In other words, chip select changes a lot during write commands.
 *
 * A write command can be canceled at any time by turning chip select to 1. But
 * it will only take effect by 1 byte boundaries, which means that if 3 bits have
 * been counted so far on the input pin, then the write command will stop
 * when 8 bits have been counted.
 *
 * For read commands, Chip select is set to 0 from 1, then instruction and address
 * are given as multiples of 8 bits. Data is being output until chip select is
 * set back to 1.
 *
 * 4.- There is a 1 byte status register:
 * SRWD - 0 - 0 - 0 - BP1 - BP2 - WEL - WIP
 *
 * WIP = 1 means a write cycle of write command is in progress, 0 otherwise.
 *    WIP can be read continuously to know when a write cycle has been
 *    completed or not.
 *
 * WEL = 1 means a write command can be executed, 0 means it cannot. It is set
 *    to 1 by a specific command, WREN. For 0 use WRDI command. Initially it is
 *    at 0, but it is also automatiically set to 0 whenever the write command
 *    has finished execution (the last write cycle has ended).
 *
 * BP1, BP0 configure the size of the memory block to be protected. WRSR command
 *    can change them unless the status register is protected.
 *
 * BP1  BP0 | Protected block | Protected array addresses
 *   0    0 |       None      |          None
 *   0    1 |   Upper quarter |      3000h - 3FFFFh
 *   1    0 |     Upper half  |       2000h - 3FFFFh
 *   1    1 |   Whole memory  |  0000h - 3FFFFh plus Identification page
 *
 * SRWD and Write Protect work together:
 * Table 4. Protection modes:
 * SRWD bit | W signal |   Status
 *    0     |   X      | Status Register is writable.
 *    1     |   1      | Status Register is write-protected.
 *    1     |   0      | Status Register is write-protected.
 * Depending on BP1, BP0, the protected addresses vary.
 *
 * 5.- There is also an identification page on the EEPROM:
 * id (3 bytes) - app params. (rest, could be used for app. data)
 * Id. field contains ST Manufactorer code, SPI Family Code, Memory Density Code.
 *
 * 6.- Instructions
 * 
 * An instruction is made up of an OPCODE followed by 3 bytes used for addresses.
 * Which bits are relevant on the adress depends on the opcode. It is important
 * to understand that the first bit transmitted is the most significant, in this
 * case the opcode will be received by the EEPROM first.
 * 
 * Instruction   | Upper address byte   |  Middle address byte    |  Lower address byte
 *  (1 byte)     | b23 b22 ... b17 b16  |  b15 b14 ... b10 b9 b8  |  b7  b6  ... b2 b1 b0
 *
 * READ or WRITE | x x ...     A17 A16  |  A15 A14 ... A10 A9 A8  |  A7 A6 ... A1 A0
 * RDID or WRID  | 0 0 ...     0   0    |  0   0  ...  0   0  0    |  A7 A6 ... A1 A0
 * RDLS or LID   | 0 0 ...     0   0    |  0   0  0 0 0 1 0  0    |  0 0   ... 0  0
 * 
 * x = irrelevant bit
 * A = relevant bit
 * 
 * Check the previously mentioned datasheet for a great explanation on the
 * sequences for each instruction.
 */

#pragma once

class MemoryEEPROM {
public:
  MemoryEEPROM() {}
  ~MemoryEEPROM() {}
  
  bool detected();

  /**
   * @brief 
   * 
   */
  void setup();

private:
};
