#include "./memory_eeprom.h"

#include <Arduino.h>
#include "SPI.h"

/**
 * WEL flag is second from the right on the byte word of the status register,
 * so apply a mask to the status register accordingly.
 */
bool MemoryEEPROM::isWriteEnabled() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_EEPROM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_EEPROM, LOW);
  SPI.transfer(RDSR_EEPROM);
  byte statusRegister = 0;
  SPI.transfer(statusRegister);
  digitalWrite(CHIP_SELECT_EEPROM, HIGH);
  SPI.endTransaction();
  return 0x02 & statusRegister == 1;
}

void MemoryEEPROM::enableWrite() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_EEPROM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_EEPROM, LOW);
  SPI.transfer(WREN_EEPROM);
  digitalWrite(CHIP_SELECT_EEPROM, HIGH);
  SPI.endTransaction();
}

void MemoryEEPROM::disableWrite() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_EEPROM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_EEPROM, LOW);
  SPI.transfer(WRDI_EEPROM);
  digitalWrite(CHIP_SELECT_EEPROM, HIGH);
  SPI.endTransaction();
}

bool MemoryEEPROM::isBusy() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_EEPROM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_EEPROM, LOW);
  SPI.transfer(RDSR_EEPROM);
  byte statusRegister = 0;
  SPI.transfer(statusRegister);
  digitalWrite(CHIP_SELECT_EEPROM, HIGH);
  SPI.endTransaction();
  return 0x01 & statusRegister == 1;
}

/**
 * When a RDSR (read status register) is executed, the status register will
 * be output constantly until chip select is put back on HIGH, so send
 * instruction once and check the output continually.
*/
void MemoryEEPROM::waitUntilReady() {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_EEPROM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_EEPROM, LOW);
  SPI.transfer(RDSR_EEPROM);
  byte statusRegister = 1;
  while (0x01 & statusRegister == 1) {
    SPI.transfer(statusRegister);
  }
  digitalWrite(CHIP_SELECT_EEPROM, HIGH);
  SPI.endTransaction();
}

uint8_t MemoryEEPROM::readByte(uint32_t address) {
  if (address > 262143) {
    Serial.print("Error: Invalid address passed to EEPROM'S readByte(address).");
    return 0;
  }
  uint8_t memoryOutputByte = 0;
  transferNBytes(READ_EEPROM, address, &memoryOutputByte, 256);
  return memoryOutputByte;
}

std::array<uint8_t, 256> MemoryEEPROM::readPage(uint32_t lowestAddress) {
  if (lowestAddress > 261888) {
    Serial.print("Error: Invalid lowestAddress passed to EEPROM'S readPage(lowestAddress).");
    return {};
  }
  std::array<uint8_t, 256> memoryOutputPage = {};
  transferNBytes(READ_EEPROM, lowestAddress, &memoryOutputPage[0], 256);
  return memoryOutputPage;
}

void MemoryEEPROM::writeByte(uint8_t byteToWrite, uint32_t address) {
  if (address > 262143) {
    Serial.print("Error: Invalid address passed to EEPROM'S writeByte(byteToWriet, address).");
    return;
  }
  transferNBytes(WRITE_EEPROM, address, &byteToWrite, 1);
}

void MemoryEEPROM::writePage(std::array<uint8_t, 256> content,
    uint32_t lowestAddress) {
  if (lowestAddress > 261888) {
    Serial.print("Error: Invalid lowestAddress passed to EEPROM'S writePage(content, address).");
    return;
  }
  transferNBytes(WRITE_EEPROM, lowestAddress, &content[0], 256);
}

/**
 * The address is of 3 bytes starting from most significant, so I apply byte wise
 * operation right shift and then convert it to byte, the result should be the
 * different bytes of the original address. Only 3 are relevant, the remaining
 * byte of address upto 32 bits is simply ignored.
 * 
 */
void MemoryEEPROM::transferNBytes(uint8_t opcode, uint32_t address, uint8_t* buffer,
    int amountOfBytes) {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_EEPROM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_EEPROM, LOW);
  SPI.transfer(opcode);
  SPI.transfer((byte)(address >> 16));
  SPI.transfer((byte)(address >> 8));
  SPI.transfer((byte)address);
  SPI.transfer(&buffer, amountOfBytes);
  digitalWrite(CHIP_SELECT_EEPROM, HIGH);
  SPI.endTransaction();
}