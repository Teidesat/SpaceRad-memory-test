/**
 * @file memory_nor_flash.h
 * @author Marcos Barrios
 * @brief Meant for a NOR Flash SPI. Model: MT25TL01GBBB8ESF-0AAT 
 * @version 0.1
 * @date 2023-09-07
 *
 * The following information comes from the datasheet:
 * https://media-www.micron.com/-/media/client/global/documents/products/data-sheet/nor-flash/serial-nor/mt25t/generation-b/mt25t_qlkt_l_01g_xbb_0.pdf
 * from the webpage:
 * https://www.mouser.es/ProductDetail/Micron/MT25TL01GBBB8ESF-0AAT?qs=rrS6PyfT74cst9NRNiyb2A%3D%3D
 *
 * Important NOTE: This NOR Flash allows 100000 ciclos ERASE mínimum.
 *
 * Two memory arrays of 512 Mbytes each. Each has sectors made up of 1024 subsectors
 *    of 32KB which are made up at the same time of 4KB subsectors.
 *
 * #### Because it is SPI based, the pins are the following:
 *    Supply voltage,
 *    Ground,
 *    Serial Clock,
 *    not(Chip select),
 *    not(RESET),
 *    not(HOLD1),
 *    not(HOLD2),
 *    not(WriteProtect1),
 *    not(WriteProtect2),
 *    I/O1[3:0],
 *    I/O2[7:4],
 *
 * There is 1 and 2 for some pins because the memory has two dies of memory array.
 *
 * Depending on the transmission mode (standard/Dual/Quad) some I/O signals will be
 * used or not.
 *
 *     Pin     | Standard  |    Dual    |    Quad   |
 * |-----------|---------- |----------- |-----------|
 * | DQ[0]     |    SI     |     IO     |     IO    |
 * | DQ[1]     |    SO     |     IO     |     IO    |
 * | DQ[2]     | <unused>  | <unused>   |     IO    |
 * | DQ[3]     | <unused>  | <unused>   |     IO    |
 * | DQ[0]     |    SI     |     IO     |     IO    |
 * | DQ[1]     |    SO     |     IO     |     IO    |
 * | DQ[2]     | <unused>  | <unused>   |     IO    |
 * | DQ[3]     | <unused>  | <unused>   |     IO    |
 *
 * #### SPI configuration:
 *
 * Transmission speed must be set to 133 MHz (133000000 on SPIConfig object),
 * so It will be used in Single Transfer Rate for the first version of the
 * project. No Dual/Quad I/O instructions used.
 *
 * Clock polarity and clock phase required for SPI communication:
 *  CPOL=0, CPHA=0 (SPI_MODE0) or
 *  CPOL=1, CPHA=1 (SPI_MODE3)
 * Make sure to configure SPI on either. Clock stays on 0 or 1 when
 * master is in stand-by-mode and not transfering data.
 *
 * ### Status Register
 *
 * Allow write to status register, select protected area defined by block
 * protect bits, write enable and write in progress are represented within
 * it.
 *
 * ### Flag status Register
 *
 * It's a register unique to this memory. Allows knowing status of write
 * operations like erase, program. Also 3 or 4 byte addressing modes.
 *
 * ### Extended Address Register
 *
 * In 3 byte addressing mode the full 512 Mbit cannot be accessed, so a register
 * is used to add an extra byte to the 3 byte address. 000000XX Depending on those
 * first 2 bits the address will be within one of the 4 memory zones within the
 * 512 MBits. They are at 0 by default.
 *
 * NOTE: When doing a READ instruction with 3 byte address mode on, then if doing
 * a continuous read, the address increments up to the last byte of the fragment,
 * wraps to the 0x00 address of the next fragment, so can read the whole memory
 * array, but the part of the address in the register does not change.
 * 
 * ### Configuration Register
 *
 * There is volatile and non volatile configuration registers with corresponding
 * instructions for writing to them. Volatile substitutes non volatile config in
 * the final configuration register (internal configuration register, it cannot be
 * accessed directly).
 *
 * El número de relojs dummy para instrucciones FAST READ, el modo eXecution-in-place
 * (XIP), el voltaje de la alimentación son cosas que pueden cambiar en el registro
 * de configuration.
 *
 * Number rof dummy clock cycles for FAST READ instructions, the eXecution-in-place
 * (XIP) mode, the supply voltage, enabling/disabling Double Transfer Rate,
 * enabling/disabling Reset/Hold at DQ3, enabling Quad I/O, enabling Dual I/O,
 * default 3-byte addressing register segment selection and the addressing
 * mode for instructions (3-byte or 4-byte) are things that can be done in the
 * non-volatile configuration register.
 *
 * Wrap type is also included in the volatile version of the configuration
 * register.
 *
 * 
 * 
 * 
 * 
 * *************** COPIED FROM NAND FLASH FROM HERE, UPDATE PENDING ***************
 * 
 * 
 * 
 * 
 * 
 * 
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

#define SPI_TRANSFER_SPEED_NAND_FLASH 133000000 // 133 MHz (Single Transfer Rate)

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
   * @pre 0 <= pageAddress <= 2^27 - 1
   * @pre length(buffer) >= 2112
   * @pre Write is enabled
   * @pre Memory is not busy
   * @pre Page to write at is not protected (TB, BP2, BP1, BP0 flags)
   * @pre Page has been erased beforehand.
   * @pre Block is being written from lowest to highest page address.
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
