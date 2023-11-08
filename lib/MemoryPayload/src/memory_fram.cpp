#include "./memory_fram.h"

#include <Arduino.h>
#include "SPI.h"

/**
 * WEL flag is second from the right on the byte word of the status register,
 * so apply a mask to the status register accordingly.
 * 
 * dummy data 0x00 is passed to transfer because I only want to read.
 */
bool MemoryFRAM::isWriteEnabled() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_FRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_FRAM, LOW);
  byte statusRegister = SPI.transfer(RDSR_FRAM);
  digitalWrite(CHIP_SELECT_FRAM, HIGH);
  SPI.endTransaction();
  return 0x02 & statusRegister == 0x02;
}

void MemoryFRAM::enableWrite() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_FRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_FRAM, LOW);
  SPI.transfer(WREN_FRAM);
  digitalWrite(CHIP_SELECT_FRAM, HIGH);
  SPI.endTransaction();
}

void MemoryFRAM::disableWrite() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_FRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_FRAM, LOW);
  SPI.transfer(WRDI_FRAM);
  digitalWrite(CHIP_SELECT_FRAM, HIGH);
  SPI.endTransaction();
}

uint8_t MemoryFRAM::readByte(size_t address) {
  if (address > 1048575 || address < 0) {
    Serial.println("Error: Invalid address passed to FRAM'S readByte(address).");
    return 0;
  }
  uint8_t memoryOutputByte = 0;
  transferNBytes(READ_FRAM, address, &memoryOutputByte, 1);
  return memoryOutputByte;
}

void MemoryFRAM::readNBytes(size_t initialAddress, uint8_t* buffer, int size) {
  if (initialAddress > 1048575 || initialAddress < 0) {
    Serial.println("Error: Invalid initialAddress passed to FRAM'S readNBytes(...).");
    return;
  }
  transferNBytes(READ_FRAM, initialAddress, buffer, size);
}

void MemoryFRAM::writeByte(uint8_t byteToWrite, size_t address) {
  if (address > 1048575 || address < 0) {
    Serial.println("Error: Invalid address passed to FRAM'S writeByte(byteToWriet, address).");
    return;
  }
  enableWrite();
  transferNBytes(WRITE_FRAM, address, &byteToWrite, 1);
}

void MemoryFRAM::writeNBytes(uint8_t* buffer, int size, size_t initialAddress) {
  if (initialAddress > 1048575 || initialAddress < 0) {
    Serial.println("Error: Invalid initialAddress passed to FRAM'S writeNBytes(...).");
    return;
  }
  enableWrite();
  transferNBytes(WRITE_FRAM, initialAddress, buffer, size);
}

/**
 * The address is of 3 bytes starting from most significant, so I apply byte wise
 * operation right shift and then convert it to byte, the result should be the
 * different bytes of the original address. Only 3 are relevant, the remaining
 * byte of address up to 32 bits is simply ignored.
 * 
 */
void MemoryFRAM::transferNBytes(uint8_t opcode, size_t address, uint8_t* buffer,
    int amountOfBytes) {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_FRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_FRAM, LOW);
  SPI.transfer(opcode);
  SPI.transfer((byte)(address >> 16));
  SPI.transfer((byte)(address >> 8));
  SPI.transfer((byte)address);
  SPI.transfer(&buffer, amountOfBytes);
  digitalWrite(CHIP_SELECT_FRAM, HIGH);
  SPI.endTransaction();
}