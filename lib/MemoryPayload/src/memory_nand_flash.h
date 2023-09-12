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
 * Important NOTE: This NAND Flash only allows 4 partial writes per page.
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
 * ### Important NOTE:
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
 * ECC-E = 1 ECC is on. (default) Read instruction checks ECC addresss of each
 *    page for data validation.
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
 * NOTE: Page with address 0 is the buffer page, which is initialized to all 0
 * on power up.
 *
 * There are specific instructions for Standard/Dual/Quad mode read/write
 *  instructions.
 *
 * Check the previously mentioned datasheet for a great explanation on the
 * sequences for each instruction.
 * 
 * I assume there is only one SPI for all the memories, so that the clock,
 *  input, output lines are all the same for the different memories, and
 *  because of that, a single SPI.begin() on the sketch will setup those
 *  lines for all the memories to use.
 */

#pragma once

#include <stdint.h> // to avoid uint8_t unknown type syntax highlight error
#include <array>
#include <Arduino.h>

// Pins
#define CHIP_SELECT_NAND_FLASH 3

// opcodes used
#define WREN_NAND_FLASH 6
#define WRDI_NAND_FLASH 4
#define RDSR_NAND_FLASH 15
#define WRSR_NAND_FLASH 1
#define READ_NAND_FLASH 3
#define PAGE_READ_NAND_FLASH 435
#define BLOCK_ERASE_NAND_FLASH 216
#define RANDOM_LOAD_PROGRAM_DATA 132
#define PROGRAM_EXECUTE 16

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
   * @brief Status register 3 has a WEL flag that at 1 allows memory to be written,
   * but at 0 it does not allow it. This changes the flag to 0.
   * 
   * Makes no effect on current write cycle; it will be successfuly finished,
   * but there won't be a next write cycle after this method is called.
   *
   */
  void disableWrite();

  /**
   * @brief The memory can be in an operation cycle, which means that a non status
   * register related instruction cannot be executed.
   *
   * This can be used to know when an operation has ended by continously calling it
   * while checking the busy flag in Status Register 3.
   *
   * @return true if memory is in an operation cycle
   * @return false if memory is not in an operation cycle
   */
  bool isBusy();

  /**
   * @brief The memory can be in a write cycle, which means that a non status
   * register related instruction cannot be executed. This sends an instruction
   * for status register read continuously to check on the BUSY flag continually
   * until it is found to be equal to 0 (ready for next instruction).
   */
  void waitUntilReady();

  /**
   * @brief Activates beyond page addressing for READ automatic address
   *    increment. READ now starts from first address of the page currently
   *    in the buffer.
   * 
   * SR-2's BUF bit=0 means that a READ instruction, which automatically
   * increments the address from the initial address, can go beyond the page's
   * maximum address instead of stopping.
   */
  void setContinuousMode();

  /**
   * @brief Activates maximum page address restriction for READ automatic adress
   *    increment. READ now allows specific address access within the page
   *    currently loaded into buffer.
   *
   * SR-2's BUF bit=1 means that a READ instruction, which automatically
   * increments the address from the initial address, stops when reaching the
   * maximum address within the page.
   */
  void setBufferMode();

  /**
   * @brief Read a single byte. Most significant is read first.
   *
   * Puts the memory in Continuous mode and reads.
   * 
   * NOTE: take into account that a page contains 64 bytes for ECC, and the
   * automatic address increment can output those 64 bytes, which are at the
   * highest address of the page.
   * 
   * TODO: check if the 64 ECC bytes can be accessed.
   *
   * @param address lower than 2^27, since the eeprom's memory array is of
   *  1 GBit.
   * @pre 0 <= address <= 2^27 - 1
   * @pre Buffer read mode is on (BUF=1 at SR-2) (required in order to access
   *    specific byte instead of always from start of page)
   */
  uint8_t readByte(size_t address);

  /**
   * @brief Read N consecutive bytes by incrementing an initialAddress
   *    N times. Most significant is read first. Includes 64 ECC bytes.
   *
   * NOTE: take into account that a page contains 64 bytes for ECC, and the
   * automatic address increment can output those 64 bytes, which are at the
   * highest address of the page.
   * 
   * TODO: check if the 64 ECC bytes are included in the output.
   *
   * @param pageAddress lower than 2^16, since thats the amount of pages
   *    in the memory array.
   * @param buffer destination of the bytes being read from the NAND.
   * @param size amount of bytes to read.
   * @pre 0 <= pageAddress <= 2^16 - 1
   * @pre length(buffer) >= 2112
   * @pre Memory is not busy
   * @pre Buffer read mode is on (BUF=1 at SR-2) (required in order to access
   *    specific byte instead of always from start of page)
   */
  void readPage(size_t pageAddress, uint8_t* buffer);

  /**
   * @brief Loads a page to the buffer. Required before READ instructions.
   * 
   * READ instructions need a page load beforehand. Call this before
   *    accesing a byte that falls within the page.
   * 
   * @param pageAddress lower than 2^16, since thats the amount of pages
   *    in the memory array.
   * @pre 0 <= pageAddress <= 2^16 - 1
   * @pre Memory is not busy
   * @post Memory is temporarily busy
   */
  void loadPageIntoBuffer(size_t pageAddress);

  /**
   * @brief Set bytes of a 128KByte Block to 0xFF and mark them as erased. It 
   *    is a requirement for a page write. The block that contaisn the addressed
   *    page is the one to be erased.
   *
   * NOTE: Write is automatically disabled after each write related instruction.
   * 
   * @param pageAddress lower than 2^16, since thats the amount of pages
   *    in the memory array.
   * @pre 0 <= pageAddress <= 2^16 - 1
   * @pre Write is enabled
   * @pre Memory is not busy
   * @pre block is unprotected (TB, BP2, BP1, BP0 flags)
   * @post Memory is temporarily busy
   */
  void eraseBlock(size_t pageAddress);

  /**
   * @brief Write A 2112 byte page. Sets page buffer to all 0xFF beforehand.
   *
   * Important NOTE: Can only be executed 4 times on a single page address.
   *    (if I understood it correctly, TODO: worth it to verify it.)
   * 
   * NOTE: Due to the bus being most significant first, then the bytes to write
   *    must be ordered in a way that takes into account that.
   * 
   * NOTE: Write is automatically disabled after each write related instruction.
   *
   * NOTE: If ECC-E = 0, the last 64 ECC bytes can be used, but if ECC-E=1 then
   * those bytes will be substituted by automatically generated ones, ignoring
   * the original content for those last 64 bytes of the page to be written.
   * 
   * @param buffer bytes that will substitute the old bytes in memory.
   * @param size amount of bytes to write.
   * @param pageAddress lower than 2^16, since thats the amount of pages
   *    in the memory array.
   * @pre 0 <= pageAddress <= 2^16 - 1
   * @pre length(buffer) >= 2112
   * @pre Write is enabled
   * @pre Memory is not busy
   * @pre Page to write at is not protected (TB, BP2, BP1, BP0 flags)
   * @pre Page has been erased beforehand.
   * @pre Block is being written from lowest to highest page address.
   * @post Page no longer in "erased" state.
   * @post Memory is temporarily busy
   */
  void writePage(uint8_t* buffer, size_t pageAddress);

private:

  /**
   * @brief Read Protection Register (SR-1), Configuration Register (SR-2) or
   *    StatusRegister(SR-3)
   * 
   * This method never fails even if memory is busy.
   * 
   * @param address either 0 or 1 or 2 depending on SR-1, SR-2 or SR-3. (Unsure
   *    if it's actually 1, 2, 3 respectively.
   * @return byte of register's content
   */
  byte readStatusRegiter(size_t address);
};
