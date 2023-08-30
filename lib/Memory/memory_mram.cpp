#include "./memory_mram.h"

#include <Arduino.h>
#include "SPI.h"

/**
 * WEL flag is second from the right on the byte word of the status register,
 * so apply a mask to the status register accordingly.
 */
bool MemoryMRAM::isWriteEnabled() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_MRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_MRAM, LOW);
  SPI.transfer(RDSR_MRAM);
  byte statusRegister = 0;
  SPI.transfer(statusRegister);
  digitalWrite(CHIP_SELECT_MRAM, HIGH);
  SPI.endTransaction();
  return 0x02 & statusRegister == 1;
}

void MemoryMRAM::enableWrite() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_MRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_MRAM, LOW);
  SPI.transfer(WREN_MRAM);
  digitalWrite(CHIP_SELECT_MRAM, HIGH);
  SPI.endTransaction();
}

void MemoryMRAM::disableWrite() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_MRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_MRAM, LOW);
  SPI.transfer(WRDI_MRAM);
  digitalWrite(CHIP_SELECT_MRAM, HIGH);
  SPI.endTransaction();
}

uint8_t MemoryMRAM::readByte(uint32_t address) {
  if (address > 1048575) {
    Serial.println("Error: Invalid address passed to MRAM'S readByte(address).");
    return 0;
  }
  uint8_t memoryOutputByte = 0;
  transferNBytes(READ_MRAM, address, &memoryOutputByte, 1);
  return memoryOutputByte;
}

void MemoryMRAM::readNBytes(uint32_t initialAddress, uint8_t* buffer, int size) {
  if (initialAddress > 1048575) {
    Serial.println("Error: Invalid initialAddress passed to MRAM'S readNBytes(...).");
    return;
  }
  transferNBytes(READ_MRAM, initialAddress, buffer, size);
}

void MemoryMRAM::writeByte(uint8_t byteToWrite, uint32_t address) {
  if (address > 1048575) {
    Serial.println("Error: Invalid address passed to MRAM'S writeByte(byteToWriet, address).");
    return;
  }
  transferNBytes(WRITE_MRAM, address, &byteToWrite, 1);
}

void MemoryMRAM::writeNBytes(uint8_t* buffer, int size, uint32_t initialAddress) {
  if (initialAddress > 1048575) {
    Serial.println("Error: Invalid initialAddress passed to MRAM'S writeNBytes(...).");
    return;
  }
  transferNBytes(WRITE_MRAM, initialAddress, buffer, size);
}

/**
 * The address is of 3 bytes starting from most significant, so I apply byte wise
 * operation right shift and then convert it to byte, the result should be the
 * different bytes of the original address. Only 3 are relevant, the remaining
 * byte of address up to 32 bits is simply ignored.
 * 
 */
void MemoryMRAM::transferNBytes(uint8_t opcode, uint32_t address, uint8_t* buffer,
    int amountOfBytes) {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_MRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_MRAM, LOW);
  SPI.transfer(opcode);
  SPI.transfer((byte)(address >> 16));
  SPI.transfer((byte)(address >> 8));
  SPI.transfer((byte)address);
  SPI.transfer(&buffer, amountOfBytes);
  digitalWrite(CHIP_SELECT_MRAM, HIGH);
  SPI.endTransaction();
}