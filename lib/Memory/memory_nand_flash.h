/**
 * @file memory_nand_flash.h
 * @author Marcos Barrios
 * @brief Meant for a NAND Flash SPI. Model: W25N01GVSFIG 
 * @version 0.1
 * @date 2023-08-30
 *
 * The following information comes from the datasheet:
 * https://www.mouser.es/datasheet/2/949/w25n01gv_revl_050918_unsecured-1489588.pdf
 * from the webpage:
 * https://www.mouser.es/ProductDetail/Winbond/W25N01GVSFIG?qs=qSfuJ%252Bfl%2Fd5wRcUOkRc5Cw%3D%3D
 *
 * The memory array is of 1 Gbit. 65536 (2^16) pages of 2048 (2^11) each ->
 *    -> 2^27 addresses total. There is an operation to delete 64 pages.
 * A block is made up of 64 pages. There are 1024 erasable blocks.
 *
 * Each page also has extra 64 bytes for error prevention Error Correction Code
 *  (ECC), check the datasheet for the specific fields in that section.
 *
 * There is one page for unique ID., another for parameter, ten more One
 *    Time Program(OTP) pages.
 *
 * ### Important note:
 * The NAND Flash allows different pin interpretations that allow standard/dual/quad
 * performance by having 2/2/4 I/O lines (with standard having one input line
 * and another output line, standard SPI that is, which is different from dual,
 * in which both lines can be either input or output).
 *
 * #### Because it is SPI based, the pins are the following:
 *    Supply voltage,
 *    Ground,
 *    Serial Clock,
 *    not(Chip select),
 *    not(HOLD) or I/O when in Quad mode,
 *    not(WriteProtect) or I/O pin when in Quad mode
 *    Serial Input or I/O pin when in Dual/Quad mode,
 *    Serial Output or I/O pin when in Dual/Quad mode,
 *
 * #### SPI configuration:
 *
 * Transmission speed must be set to 104 MHz (104000000 on SPIConfig object)
 *
 * Clock polarity and clock phase required for SPI communication:
 *  CPOL=0, CPHA=0 (SPI_MODE0) or
 *  CPOL=1, CPHA=1 (SPI_MODE3)
 * Make sure to configure SPI on either. Clock stays on 0 or 1 when
 * master is in stand-by-mode and not transfering data.
 *
 * Memory operations are done by instructions.
 *
 * How to write and read:
 *
 * For read instructions, Chip select is set to 0 from 1, then Page Data Read
 * instruction is called, then Chip select back to 1. Then execute the read
 * instruction. Depending on the read mode (Buffer or Continuous, check
 * configuration register), the byte address within page will be relevant
 * or not, and the memory will stop outputting when reaching the end of the
 * addressed page or continue with the next page. Either way, put chip select
 * back to 1 to stop the execution of the instruction. After a read instruction,
 * it is ideal to check ECC-1 and ECC-0 bits from the status register to verify
 * the data integrity (Check table for meaning for the 4 different combinations),
 * data might not be usable.
 * 
 * WIP Chip select must be 1 before a write command. It has to be first
 * set to 0, and then a WREN (Write Enable) command which must be executed
 * first. When looking to end an instruction, Chip select must be put back
 * to 1.
 *
 * #### There are 3 status registers of 1 byte each.
 *
 * They are named SR-1, SR-2 and SR-3. Accessed by a 1 byte address and a
 * specific instruction for reading and writing to status register.
 *
 * ### SR-1 (Protection Register)
 * 
 * SRP0 - BP1 - BP2 - BP3 - BP4 - TB - WP-E - SRP1
 *
 * All are volatile; on power loss the current value is lost. But can be One Time
 * Program'd (OTP) into non volatile and read-only.
 * 
 * It contains a WP-E bit that configures Software Protection (SP) or Hardware
 *    Protection (HP).
 *
 * Depending on the two SRP bits in the status register, the WP will have a
 *  function or another.
 *
 *  WP-E = 0 then SP mode. Write Protect (WP) pin becomes I/O for Quad
 *    instructions, which is enabled.
 *  WP-E = 1 then HP mode. WP = 1 means the whole memory is write protected,
 *  cannot write, program, erase on any part of the memory. Also, WP cannot
 *  be used as I/O for Quad mode, Quad mode is completly disabled.
 * 
 *  BP1,BP2,BP3,BP4,TB = All, none, or a portion of the memory array cannot
 *    be applied Program or Erase instructions. Default = 1 on all after
 *    power-up, which means all of the memory is protected by default.
 *    Although depending on another bit in SR-2 the default can be changed
 *    to a OTP value.
 *  SRP0, SRP1 = Controls protection modes and configuration. See the datasheet
 *  for the detailed tables on 7.1.3 section.
 *
 * ### SR-2 (Configuration Register)
 *
 * OTP-L - OTP-E - SR1-L - ECC-E - BUF - (R)* - (R) - (R)
 * *(R) = Reserved
 *
 * Only OTP-L and SR1-L are OTP lockable, the rest are set during operation
 * or on power-up at default value.
 *
 * OTP-L = 0 means that the 10 OTP pages are yet to be locked permanently.
 * OTP-L = 1 means that the 10 OTP pages have been locked permanently and
 *    cannot be changed.
 *
 * OTP-E = 0 means unique id and other special pages cannot be operated with.
 * OTP-E = 1 means unique id and other special pages CAN be operated with.
 *
 * SR1-L = 0 the SR-1 register is not OTP locked.
 *       = 1 the SR-1 register IS OTP locked. (Requires SRP1-SRP2 on SR-1 to be
 *       equal to (1, 1), and OTP-E=1 beforehand)
 *
 * ECC-E = 0 ECC is off.
 * ECC-E = 0 ECC is on. Read instruction checks ECC addresss of each page
 *    for data validation.
 *
 * BUFF = 0 means Buffer Read Mode. Partial page read starting from byte address
 *    within block. Stops after page fully read (output line to high impedance).
 * BUFF = 1 means Continuos Read Mode; full page read independently of the byte
 *    address within block. Next page after finish current page.
 * 
 * ### SR-3 (Status Only Register)
 * 
 * (R)* - LUT-F - ECC-1 - ECC-0 - P-FAIL - E-FAIL - WEL - BUSY
 * *(R) = reserved
 * 
 * LUT-F = 0 Look-Up Table is not fully used. Related to bad block management.
 * LUT-F = 1 Look-Up Table IS fully used. Related to bad block management.
 * 
 * ECC-1 = 0, ECC-0 = 0 -> Entire data output is successful, without any ECC correction
 * ECC-1 = 0, ECC-0 = 1 -> Entire data output is successful, but some ECC corrections.
 * ECC-1 = 1, ECC-0 = 0 -> more than 4 errors in single page, cannot correct. ... (more)
 * ECC-1 = 1, ECC-0 = 1 -> more than 4 errors in many pages, cannot correct. ... (more)
 * 
 * P-FAIL = 0 Program instruction didn't fail. if = 1 then it did fail.
 * E-FAIL = 0 Erase instruction didn't fail. if = 1 then it did fail.
 * Both of these can be at 0 if attempted on locked, protected or OTP address.
 *
 * WEL = 1 means modification instructions can be executed, 0 means they cannot.
 *    It is set to 1 by a specific command, WREN. For 0 use WRDI command.
 *    Initially it is at 0, but it is also automatically set to 0 on certain
 *    instruction executions or manually by using a write disable instruction.
 *
 * BUSY = 1 means some operation is in process in the memory. When the operation
 *    is completed, it is set automatically to 0, so it can be used to know when
 *    a new instruction can be passed to the memory. Read Status Register and
 *    Read JEDEC ID instructions can be executed even if currently busy.
 *
 * #### Instructions
 *
 * An instruction is made up of an OPCODE followed by address or dummy bytes
 * depending on the specific instruction.It is important to understand that
 * the first bit transmitted is the most significant, in this case the opcode
 * will be received by the NAND Flash first.
 *
 * There are 27 instructions, so they are not included in this comment block
 * due to length optimization.
 *
 * An address can be:
 * A) x x x x - Block-Address(10 bits) - Page-Address(6 bits) - Byte-Address(11 bits)
 * B) x x x x -           Page-Address(16 bits)               - Byte-Address(11 bits)
 * x = irrelevant bit
 *
 * Note: Page with address 0 is the buffer page, which is initialized to all 0
 * on power up.
 *
 * There are specific instructions for Standard/Dual/Quad mode read/write
 *  instructions.
 *
 * Check the previously mentioned datasheet for a great explanation on the
 * sequences for each instruction.
 */

#pragma once

#include <stdint.h> // to avoid uint8_t unknown type syntax highlight error
#include <array>

// Pins
#define CHIP_SELECT_NAND_FLASH 3

// I assume there is only one SPI for all the memories, so that the clock,
// input, output lines are all the same for the different memories, and
// because of that, a single SPI.begin() on the sketch will setup those
// lines for all the memories to use.

// opcodes
// WIP
#define WREN_NAND_FLASH 6
#define WRDI_NAND_FLASH 4
#define RDSR_NAND_FLASH 5
#define WRSR_NAND_FLASH 1
#define READ_NAND_FLASH 3
#define WRITE_NAND_FLASH 2
#define SLEEP_NAND_FLASH 185
#define WAKE_NAND_FLASH 171

#define SPI_TRANSFER_SPEED_NAND_FLASH 104000000 // 104 MHz

class MemoryNANDFlash {
public:
  MemoryNANDFlash() {}
  ~MemoryNANDFlash() {}

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
   * @pre Write is enabled
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
   * @pre Write is enabled
   * @pre Region to write at is not protected.
   */
  void writeNBytes(uint8_t* buffer, int size, uint32_t initialAddress);

private:
  // because readByte, readPage, writeByte, writePage are similar and will
  // likely stay similar. So this is a auxiliary function for them.
  void transferNBytes(uint8_t opcode, uint32_t address, uint8_t* buffer,
      int amountOfBytes);
};
