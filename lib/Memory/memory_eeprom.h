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
 * #### Because it is SPI based, the pins are the following:
 *    Supply voltage,
 *    Ground,
 *    Serial Clock,
 *    not(Chip select),
 *    not(HOLD),
 *    not(WriteProtect),
 *    Serial Input,
 *    Serial Output
 *
 * When HOLD is 0 the memory goes into stand-by mode and the output
 * stays at high impendance, whiel also ignoring input from the bus.
 *
 * #### SPI configuration:
 *
 * Transmission speed must be set to either 10 MHz (10000000 on SPIConfig object) or
 * 5 MHz depending on power voltage. If greater than 2.5V then set 5MHz, on the
 * other han, if greater than 4.5V then set 10MHz.
 * 
 * Clock polarity and clock phase required for the SPI communication:
 *  CPOL=0, CPHA=0 or
 *  CPOL=0, CPHA=1
 * Make sure to configure SPI on either. Clock stays on 0 or 1 when
 * master is in stand-by-mode and not transfering data.
 *
 * #### Memory operations are done by commands:
 *  - WRITE, WRSR, WRID, LID...
 *  - READ, RDSR, RDID, RDLS...
 *  etc...
 *
 * How to write and read:
 *
 * Chip select must be 1 before a write command. It has to be first
 * set to 0, and then a WREN (Write Enable) command must be first executed.
 * When aiming to end an instruction, Chip select must be put back to 1.
 *
 * A write instruction can be canceled at any time by turning chip select to 1.
 * But it will only take effect by 1 byte boundaries, which means that if 3 bits
 * have been counted so far on the input pin, then the write instruction will
 * stop when 8 bits have been counted.
 *
 * For read commands, Chip select is set to 0 from 1, then instruction and address
 * are given as multiples of 8 bits. Data is being output until chip select is
 * set back to 1.
 *
 * #### There is a 1 byte status register:
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
 *   1    0 |     Upper half  |      2000h - 3FFFFh
 *   1    1 |   Whole memory  |  0000h - 3FFFFh plus Identification page
 *
 * SRWD and Write Protect work together:
 * Table: Protection modes:
 * SRWD bit | W signal |   Status
 *    0     |   X      | Status Register is writable.
 *    1     |   1      | Status Register is write-protected.
 *    1     |   0      | Status Register is write-protected.
 * Depending on BP1, BP0, the protected addresses vary.
 *
 * #### There is also an identification page on the EEPROM:
 * id (3 bytes) - app params. (rest, could be used for app. data)
 * Id. field contains ST Manufacturer code, SPI Family Code, Memory Density Code.
 *
 * #### Instructions
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

#include <stdint.h> // to avoid uint8_t unknown type syntax highlight error
#include <array>

// Pins
#define CHIP_SELECT_EEPROM 3

// I assume there is only one SPI for all the memories, so that the clock,
// input, output lines are all the same for the different memories, and
// because of that, a single SPI.begin() on the sketch will setup those
// lines for all the memories to use.

// opcodes
#define WREN_EEPROM 6
#define WRDI_EEPROM 4
#define RDSR_EEPROM 5
#define WRSR_EEPROM 1
#define READ_EEPROM 3
#define WRITE_EEPROM 2
#define RDID_EEPROM 131 // Same number for RDLS instruction (read id page lock status)
#define WRID_EEPROM 130 // Same number for LID (lock id page in read only)
// #define RDLS 131
// #define LID 130

#define SPI_TRANSFER_SPEED_EEPROM 5000000 // 5 Hz assuming 3.3 V

class MemoryEEPROM {
public:
  MemoryEEPROM() {}
  ~MemoryEEPROM() {}
  
  /**
   * @brief Perform a RDSR read status register instruction. Check if the WEL
   * flag in the status register is at 1 (allow write instructions) or at 0
   * (dissallow write instructions)
   * 
   * Cannot fail because it simply checks status register by a RDSR instruction.
   * 
   * @return true if WEL = 1
   * @return false if WEL = 0
   */
  bool isWriteEnabled();

  /**
   * @brief Status register has a WEL flag that at 1 allows memory to be written,
   * but at 0 it does not allow it. This changes the flag to 1.
   * 
   * Note: this will fail if the memory is currently in a write cycle.
   * @pre Memory not busy
   */
  void enableWrite();

  /**
   * @brief Status register has a WEL flag that at 1 allows memory to be written,
   * but at 0 it does not allow it. This changes the flag to 0.
   * 
   * Makes no effect on current write cycle; it will be successfuly finished,
   * but there won't be a next write cycle after this method is called.
   * 
   * Note: this will fail if the memory is currently in a write cycle.
   * @pre Memory not busy
   */
  void disableWrite();

  /**
   * @brief The memory can be in a write cycle, which means that a non status
   * register related instruction cannot be executed.
   * 
   * Maybe instead of waiting until ready, the user wants to do something
   * inbetween each check for busy, thus this method.
   * 
   * @return true if memory is in a write cycle
   * @return false if memory is not in a write cycle
   */
  bool isBusy();

  /**
   * @brief The memory can be in a write cycle, which means that a non status
   * register related instruction cannot be executed. This sends an instruction
   * for status register read continously to check on the WIP flag continually
   * until it is found to be equal to 0 (ready for instruction).
   *
   */
  void waitUntilReady();

  /**
   * @brief read a single byte.
   * 
   * Note: when the memory is busy writing something, a read cannot be performed,
   * so make sure to check if memory is busy beforehand.
   * 
   * @param address lower than 2^18, since the eeprom's memory array is of
   *  256 Kbyte.
   * @pre 0 <= address <= 2^18 - 1
   * @pre Memory is not busy
   */
  uint8_t readByte(uint32_t address);

  /**
   * @brief read a page.
   * 
   * In this EEPROM, pages are of size 256 byte.
   * 
   * Note: when the memory is busy writing something, a read cannot be performed,
   * so make sure to check if memory is busy beforehand.
   * 
   * @param lowestAddress lower than (2^18 - 255), since the eeprom's memory
   * array is of 256 Kbyte, and the address is incremented 255 times to be able
   * to read the whole 256 byte page.
   * @pre 0 <= lowestAddress <= ((2^18 - 1) - 255)
   */
  std::array<uint8_t, 256> readPage(uint32_t lowestAddress);

  /**
   * @brief Write a byte.
   * 
   * @param uint8_t byteToWrite
   * @param address lower than 2^18, since the eeprom's memory array is of
   *  256 Kbyte.
   * @pre 0 <= address <= 2^18 - 1
   * @pre Memory not busy.
   * @pre Region to write at is not protected.
   */
  void writeByte(uint8_t byteToWrite, uint32_t address);

  /**
   * @brief write a page with a single internal Write cycle.
   * 
   * In this EEPROM, pages are of size 256 byte.
   * 
   * @param content bytes that will substitute the old bytes in memory. 
   * @param lowestAddress lower than (2^18 - 255), since the eeprom's memory
   * array is of 256 Kbyte, and the address is incremented 255 times to be able
   * to read the whole 256 byte page.
   * @pre 0 <= lowestAddress <= ((2^18 - 1) - 255)
   * @pre Memory not busy.
   * @pre Region to write at is not protected.
   */
  void writePage(std::array<uint8_t, 256> content, uint32_t lowestAddress);

private:
  // because readByte, readPage, writeByte, writePage are similar and will
  // likely stay similar. So this is a auxiliary function for them.
  void transferNBytes(uint8_t opcode, uint32_t address, uint8_t* buffer,
      int amountOfBytes);
};
