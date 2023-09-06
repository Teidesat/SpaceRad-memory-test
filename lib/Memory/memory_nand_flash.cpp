#include "./memory_nand_flash.h"

#include <Arduino.h>
#include "SPI.h"

/**
 * WEL flag is second from the right on the byte word of the StatusRegister-3,
 * so apply a mask to the status register accordingly.
 * 
 * dummy data 0x00 is passed to transfer because I only want to read.
 */
bool MemoryNANDFlash::isWriteEnabled() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(RDSR_NAND_FLASH);
  SPI.transfer(0x02); // Unsure if SR Address 2 is for SR-2 or for SR-3, but want SR-3.
  byte statusRegister = SPI.transfer(0x00);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
  return 0x02 & statusRegister == 0x02;
}

void MemoryNANDFlash::enableWrite() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(WREN_NAND_FLASH);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
}

void MemoryNANDFlash::disableWrite() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(WRDI_NAND_FLASH);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
}

bool MemoryNANDFlash::isBusy() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(RDSR_NAND_FLASH);
  byte statusRegister = 0;
  SPI.transfer(statusRegister);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
  return 0x01 & statusRegister == 0x01;
}

/**
 * When a RDSR (read status register) is executed, the status register will
 * be output constantly until chip select is put back on HIGH, so send
 * instruction once and check the output continually.
*/
void MemoryNANDFlash::waitUntilReady() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(RDSR_NAND_FLASH);
  byte statusRegister = SPI.transfer(0x00);
  while (0x01 & statusRegister == 0x01) {
    SPI.transfer(statusRegister);
  }
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
}

// apply 11111110 mask because last bit of configRegister is the busy one, and
// I want it at 0.
// Unsure if SR Address 1 is for SR-1 or for SR-2, but want 2.
void MemoryNANDFlash::setContinuousMode() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  byte configRegister = readStatusRegiter(1);
  delay(10); // unsure if necessary, but it's a high to low immediately
  byte newConfigRegister = (configRegister & 0xFE);
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(WRSR_NAND_FLASH);
  SPI.transfer(0x01); 
  SPI.transfer(newConfigRegister);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
}

// OR operation 00000001 because last bit of configRegister is the busy one, and
// I want it at 1.
// Unsure if SR Address 1 is for SR-1 or for SR-2, but want SR-2.
void MemoryNANDFlash::setBufferMode() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  byte configRegister = readStatusRegiter(1);
  delay(10); // unsure if necessary, but it's a high to low immediately
  byte newConfigRegister = (configRegister | 0x01);
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(WRSR_NAND_FLASH);
  SPI.transfer(0x01);
  SPI.transfer(newConfigRegister);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
}

// Out of 27 relevant bits of an address, 16 are page address, 11 byte addresses
// within page, so I pass the 16 most significant address bits to the page load
// into buffer function.
//
// loadPageIntoBuffer(pageAddress (address >> 11 in this case)); must be called
// beforehand.
uint8_t MemoryNANDFlash::readByte(size_t address) {
  if (address > 134217727 || address < 0) {
    Serial.println("Error: Invalid address passed to NAND_FLASH's readByte(address).");
    return 0;
  }
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(READ_NAND_FLASH);
  SPI.transfer16(address);
  SPI.transfer(0x00); // dummy
  byte outputByte = SPI.transfer(0x00);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
  return outputByte;
}

// 11 shifts to left so that it starts at the first address of the page.
// Also, 2112 is page length (2048 + 64 bytes of ecc)
void MemoryNANDFlash::readPage(size_t pageAddress, uint8_t* buffer) {
  if (pageAddress > 65535 || pageAddress < 0) {
    Serial.println("Error: Invalid pageAddress passed to MRAM's readNBytes(...).");
    return;
  }
  loadPageIntoBuffer(pageAddress);
  waitUntilReady();
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(READ_NAND_FLASH);
  SPI.transfer16(pageAddress << 11);
  SPI.transfer(0x00); // dummy
  SPI.transfer(buffer, 2112);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
}

void MemoryNANDFlash::eraseBlock(size_t pageAddress) {
  if (pageAddress > 1023 || pageAddress < 0) {
    Serial.println("Error: Invalid pageAddress passed to NAND Flash's eraseBlock(...).");
  }
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(BLOCK_ERASE_NAND_FLASH);
  SPI.transfer(0x00); // dummy
  SPI.transfer16(pageAddress);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
}

// 2112 bytes is the size of a page (2^11 bytes + ECC's 64 bytes)
// I do random load instead of load just in case some byte to write is wrong,
// in which case, the random version sets it to 0xFF. normal load works too.
void MemoryNANDFlash::writePage(uint8_t* buffer, size_t pageAddress) {
  if (pageAddress > 65535 || pageAddress < 0) {
    Serial.println("Error: Invalid pageAddress passed to NAND Flash's writePage(...).");
    return;
  }
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(RANDOM_LOAD_PROGRAM_DATA);
  SPI.transfer(0x00); // dummy
  SPI.transfer16(0x00); // start from address 0 of buffer page
  SPI.transfer(buffer, 2112);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  delay(10); // unsure if needed
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(PROGRAM_EXECUTE);
  SPI.transfer(0x00); // dummy
  SPI.transfer16(pageAddress);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
}

byte MemoryNANDFlash::readStatusRegiter(size_t address) {
  if (address > 3 || address < 0) {
    Serial.println("Error: Invalid adddress, NAND FLASH'S readStatusRegister().");
    return 0x00;
  }
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(RDSR_NAND_FLASH);
  SPI.transfer(address);
  byte registerContent = SPI.transfer(0x00); // dummy
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
  return registerContent;
}

/**
 * send opcode byte, then dummy byte, then extract page address for a buffer
 * page read which is required before accessing the specific byte, so shift
 * 11 bits to the right (27 relevant bit address). Then the buffer is
 * ready for reading after the BUSY flag is back to 0.
 */
void MemoryNANDFlash::loadPageIntoBuffer(size_t pageAddress) {
  if (pageAddress > 65535 || pageAddress < 0) {
    Serial.println("Error: Invalid pageAddress passed to NAND Flash's loadPageIntoBuffer(...).");
    return;
  }
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  SPI.transfer(PAGE_READ_NAND_FLASH);
  SPI.transfer(0x00);
  SPI.transfer16(pageAddress);
  digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  SPI.endTransaction();
}
