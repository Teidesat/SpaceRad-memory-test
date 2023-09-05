#include "./memory_nand_flash.h"

#include <Arduino.h>
#include "SPI.h"

// ******************* NOT UPDATED, COPIED FUNCTIONS FROM OTHER MEMORIES.

/**
 * WEL flag is second from the right on the byte word of the status register,
 * so apply a mask to the status register accordingly.
 */
bool MemoryNANDFlash::isWriteEnabled() {
  // SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  // digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  // SPI.transfer(RDSR_NAND_FLASH);
  // byte statusRegister = 0;
  // SPI.transfer(statusRegister);
  // digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  // SPI.endTransaction();
  // return 0x02 & statusRegister == 1;
}

void MemoryNANDFlash::enableWrite() {
  // SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  // digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  // SPI.transfer(WREN_NAND_FLASH);
  // digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  // SPI.endTransaction();
}

void MemoryNANDFlash::disableWrite() {
  // SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  // digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  // SPI.transfer(WRDI_NAND_FLASH);
  // digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  // SPI.endTransaction();
}

bool MemoryNANDFlash::isBusy() {
  // SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  // digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  // SPI.transfer(RDSR_NAND_FLASH);
  // byte statusRegister = 0;
  // SPI.transfer(statusRegister);
  // digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  // SPI.endTransaction();
  // return 0x01 & statusRegister == 1;
}

/**
 * When a RDSR (read status register) is executed, the status register will
 * be output constantly until chip select is put back on HIGH, so send
 * instruction once and check the output continually.
*/
void MemoryNANDFlash::waitUntilReady() {
  // SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  // digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  // SPI.transfer(RDSR_NAND_FLASH);
  // byte statusRegister = 1;
  // while (0x01 & statusRegister == 1) {
  //   SPI.transfer(statusRegister);
  // }
  // digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  // SPI.endTransaction();
}

uint8_t MemoryNANDFlash::readByte(uint32_t address) {
  // if (address > 262143) {
  //   Serial.println("Error: Invalid address passed to NAND_FLASH'S readByte(address).");
  //   return 0;
  // }
  // uint8_t memoryOutputByte = 0;
  // transferNBytes(READ_NAND_FLASH, address, &memoryOutputByte, 1);
  // return memoryOutputByte;
}

void MemoryNANDFlash::readNBytes(uint32_t initialAddress, uint8_t* buffer, int size) {
  // if (initialAddress > 1048575) {
  //   Serial.println("Error: Invalid initialAddress passed to MRAM'S readNBytes(...).");
  //   return;
  // }
  // transferNBytes(READ_NAND_FLASH, initialAddress, buffer, size);
}

// TODO: check if write enable can apply wwhen memory is not busy or not,
// in which case an additional check for isBusy() is required beforehand.
void MemoryNANDFlash::writeByte(uint8_t byteToWrite, uint32_t address) {
  // if (address > 262143) {
  //   Serial.println("Error: Invalid address passed to NAND_FLASH'S writeByte(byteToWriet, address).");
  //   return;
  // }
  // enableWrite();
  // transferNBytes(WRITE_NAND_FLASH, address, &byteToWrite, 1);
}

void MemoryNANDFlash::writeNBytes(uint8_t* buffer, int size, uint32_t initialAddress) {
  // if (initialAddress > 1048575) {
  //   Serial.println("Error: Invalid initialAddress passed to MRAM'S writeNBytes(...).");
  //   return;
  // }
  // transferNBytes(WRITE_NAND_FLASH, initialAddress, buffer, size);
}

/**
 * The address is of 3 bytes starting from most significant, so I apply byte wise
 * operation right shift and then convert it to byte, the result should be the
 * different bytes of the original address. Only 3 are relevant, the remaining
 * byte of address upto 32 bits is simply ignored.
 * 
 */
void MemoryNANDFlash::transferNBytes(uint8_t opcode, uint32_t address, uint8_t* buffer,
    int amountOfBytes) {
  // SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_NAND_FLASH, MSBFIRST, SPI_MODE0));
  // digitalWrite(CHIP_SELECT_NAND_FLASH, LOW);
  // SPI.transfer(opcode);
  // SPI.transfer((byte)(address >> 16));
  // SPI.transfer((byte)(address >> 8));
  // SPI.transfer((byte)address);
  // SPI.transfer(&buffer, amountOfBytes);
  // digitalWrite(CHIP_SELECT_NAND_FLASH, HIGH);
  // SPI.endTransaction();
}