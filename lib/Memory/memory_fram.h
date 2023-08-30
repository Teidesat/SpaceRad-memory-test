/**
 * @file memory_fram.h
 * @author Marcos Barrios
 * @brief Meant for a FRAM Flash SPI. Model: CY15B108QN-40SXI 
 * @version 0.1
 * @date 2023-08-29
 *
 * The following information comes from the datasheet:
 * https://www.mouser.es/datasheet/2/196/Infineon_CY15B108QN_CY15V108QN_Excelon_LP_4_Mbit__-3161185.pdf
 * from the webpage:
 * https://www.mouser.es/ProductDetail/Infineon-Technologies/CY15B108QN-40SXI?qs=P1JMDcb91o4DSS%2FkAQM5uA%3D%3D
 *
 * The datasheet is in chinese, and to write this comment a translation process
 * took place, so until this class is actually tested on real hardware, some
 * things might be wrong due to missunderstandings due to that translation.
 *
 * The memory array is of 8Mbit including a special sector of 256 bytes.
 *
 * #### Because it is SPI based, the pins are the following:
 *    Supply voltage,
 *    Ground,
 *    Serial Clock,
 *    not(Chip select),
 *    not(RESET),
 *    not(WriteProtect),
 *    Serial Input,
 *    Serial Output
 *
 * #### SPI configuration:
 *
 * Transmission speed must be set to  8 MHz (8000000 on SPIConfig object)
 *
 * Clock polarity and clock phase required for the SPI communication:
 *  CPOL=0, CPHA=0 (SPI_MODE0) or
 *  CPOL=1, CPHA=1 (SPI_MODE3)
 * Make sure to configure SPI on either. Clock stays on 0 or 1 when
 * master is in stand-by-mode and not transfering data.
 *
 * #### Memory operations are done by commands:
 *  WREN = Write enable
 *  WRDI = Write disable
 *  RDSR = Read status register
 *  WRSR = Write status register
 *  WRITE
 *  READ
 *  FSTRD = Fast read
 *  SSWR = Write special sector
 *  SSRD = Read special sector
 *  RDID = Read device id.
 *  RUID = Read unique id.
 *  WRSN = Write serial number
 *  RDSN = Read serial number
 *  DPD = Change to deep shutdown mode (0.86 mA instead of 600 mA
 *    or 2.95 mA if on standby)
 *  HBN = Change to suspension mode (0.1 mA instead of 600 mA
 *    or 2.95 mA if on standby)
 *
 * How to write and read:
 * 
 * Chip select must be 1 before a write command. It has to be first
 * set to 0, and then a WREN (Write Enable) command must be first executed.
 * When aiming to end an instruction, Chip select must be put back to 1.
 *
 * Unline EEPROM, in this FRAM during a write instruction, when a byte
 * reaches the memory, it is written directly instead of having to wait
 * a write cycle.
 *
 * For read commands, Chip select is set to 0 from 1, then instruction and
 * address are given as multiples of 8 bits. Data is output until chip
 * select is set back to 1.
 *
 * #### There is a 1 byte status register:
 * WPEN - 1 - 0 - 0 - BP1 - BP2 - WEL - 0
 *
 * There is no WIP flag for memory busy signaling during write cycles because
 * the FRAM writes in real time and does not require an extra write cycle like
 * an EEPROOM does.
 * 
 * WEL = 1 means a write command can be executed, 0 means it cannot. It is set
 *    to 1 by a specific command, WREN. For 0 use WRDI command. Initially it is
 *    at 0, but it is also automatically set to 0 whenever the write command
 *    has finished execution (the last write cycle has ended).
 *    The WEL flag can only by changed by WREN and WRDI instructions.
 *
 * BP1, BP0 configure the size of the memory block to be protected. WRSR command
 *    can change them unless the status register is protected.
 *
 * BP1  BP0 | Protected block | Protected array addresses
 *   0    0 |       None      |          None
 *   0    1 |   Upper quarter |      C0000h - FFFFFh （Top 1/4）
 *   1    0 |     Upper half  |      80000h to FFFFFh (Top 1/2)
 *   1    1 |   Whole memory  |      00000h to FFFFFh (All Addresses)
 *
 * SRWD and Write Protect work together:
 * Table: Write Protection
 * WEL | WPEN | WP | Protected Modules | Unprotected Modules | Status Register
 * 0   |  X   | X  |    Protected      |    Protected        |    Protected
 * 1   |  0   | X  |    Protected      |    Unprotected      |    Unprotected
 * 1   |  1   | 0  |    Protected      |    Unprotected      |    Protected
 * 1   |  1   | 1  |    Protected      |    Unprotected      |    Unprotected
 * Depending on BP1, BP0, the protection varies.
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
 * READ or WRITE | x x A19 A18 A17 A16  |  A15 A14 ... A10 A9 A8  |  A7 A6 ... A1 A0
 * RDID or WRID  | 0 0 ...     0   0    |  0   0  ...  0   0  0    |  A7 A6 ... A1 A0
 * RDLS or LID   | 0 0 ...     0   0    |  0   0  0 0 0 1 0  0    |  0 0   ... 0  0
 *
 * x = irrelevant bit
 * A = relevant bit
 *
 * A FRAM's write instruction can write consecutively without any page limits
 * that the EEPROM has (up to 256 bytes, which is it's page size). Which
 * means, that when a Write byte is sent then consecutive bytes can be sent
 * and the address will increment for each byte up to FFFFFh, after which it
 * resets to 00000h, so the most significant byte is written first.
 * 
 * A READ does the same; consecutively reading and increasing upto FFFFFh
 * then resetting to 00000h to keep incrementing.
 * 
 * Check the previously mentioned datasheet for a great explanation on the
 * sequences for each instruction. Although it is in chinese.
 */

#pragma once

#include <stdint.h> // to avoid uint8_t unknown type syntax highlight error
#include <array>

// Pins
#define CHIP_SELECT_FRAM 3

// I assume there is only one SPI for all the memories, so that the clock,
// input, output lines are all the same for the different memories, and
// because of that, a single SPI.begin() on the sketch will setup those
// lines for all the memories to use.

// opcodes
#define WREN_FRAM 6
#define WRDI_FRAM 4
#define RDSR_FRAM 5
#define WRSR_FRAM 1
#define WRITE_FRAM 2
#define READ_FRAM 3
#define FSTRD_FRAM 11
#define SSWR_FRAM 66
#define SSRD_FRAM 75
#define RDID_FRAM 159
#define RUID_FRAM 76 
#define WRSN_FRAM 194
#define RDSN_FRAM 195
#define DPD_FRAM 186
#define HBN_FRAM 185

#define SPI_TRANSFER_SPEED_FRAM 8000000 // 8 Hz typical

class MemoryFRAM {
public:
  MemoryFRAM() {}
  ~MemoryFRAM() {}
  
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
   */
  void enableWrite();

  /**
   * @brief Status register has a WEL flag that at 1 allows memory to be written,
   * but at 0 it does not allow it. This changes the flag to 0.
   * 
   * Makes no effect on current write cycle; it will be successfuly finished,
   * but there won't be a next write cycle after this method is called.
   * 
   */
  void disableWrite();

  /**
   * @brief Read a single byte.
   * 
   * @param address lower than 2^20, since the eeprom's memory array is of
   *  8 MBit.
   * @pre 0 <= address <= 2^20 - 1
   */
  uint8_t readByte(uint32_t address);

  /**
   * @brief Read N consecutive bytes by incrementing an initialAddress
   *    N times.
   *
   * @param initialAddress lower than 2^20, since the FRAM's memory
   *  array is of 8 MBit. If initialAddress + size > 2^20 then a reset
   *  to 0 takes place and it keeps going from there.
   * @param buffer destination of the bytes being read from the FRAM.
   * @param size amount of bytes to read.
   * @pre 0 <= initialAddress <= 2^20 - 1
   */
  void readNBytes(uint32_t initialAddress, uint8_t* buffer, int size);

  /**
   * @brief Write a byte.
   * 
   * Note: write needs to be enabled for the instruction to take effect.
   * 
   * Also, if power goes down, the last incompleted byte to write will be lost.
   *
   * @param uint8_t byteToWrite
   * @param address lower than 2^20, since the eeprom's memory array is of
   *  8 MBit.
   * @pre 0 <= address <= 2^20 - 1
   * @pre Region to write at is not protected.
   */
  void writeByte(uint8_t byteToWrite, uint32_t address);

  /**
   * @brief Write N consecutive bytes by incrementing an initialAddress
   *    N times.
   * 
   * Note: due to the bus being most significant first, then write most
   *    significant first.
   * 
   * Also, if power goes down, the last incompleted byte to write will be lost.
   * 
   * Note: write needs to be enabled for the instruction to take effect.
   * 
   * @param buffer bytes that will substitute the old bytes in memory.
   * @param size amount of bytes to write.
   * @param initialAddress lower than 2^20, since the FRAM's memory
   *  array is of 8 MBit. If initialAddress + size > 2^20 then a reset
   *  to 0 takes place and it keeps going from there.
   * @pre 0 <= address <= 2^20 - 1
   * @pre Region to write at is not protected.
   */
  void writeNBytes(uint8_t* buffer, int size, uint32_t initialAddress);

  // Consecutive reads. there is a fast-read instruction version that
  // adds a dummy byte that cannot be 1010XXXX, so 5 bytes total instead
  // of 4 to execute that instruction.
  // TODO: void fastRead(...);
private:
  // because readByte, readPage, writeByte, writePage are similar and will
  // likely stay similar. So this is a auxiliary function for them.
  void transferNBytes(uint8_t opcode, uint32_t address, uint8_t* buffer,
      int amountOfBytes);
};
