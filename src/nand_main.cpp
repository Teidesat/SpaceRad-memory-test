/**
 * @file nand_test.cpp
 * @author Marcos Barrios
 * @brief Meant to test whether NAND's pins are properly connected.
 * @version 0.1
 * @date 2023-09-12
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <Arduino.h>
#include <memory_nand_flash.h>

#include "SPI.h"

// **** first update chip select pins on the class ****

MemoryNANDFlash nand;

Array<uint8_t, 2112> obtainedPage = {};

void setup() {
  SPI.begin();
  Serial.begin(9600);
  delay(5); // after 5 ms device is fully accessible
  nand.enableWrite();
  delay(1);
  Array<uint8_t, 2112> pageToWrite = {};
  for (size_t i = 0; i < 2112; ++i) { // I want to change values to any non default
    pageToWrite[i] = (i + 1) % 256;
  }

  // (2112 bytes/page * 64 pages/block) / 8 bits = 16896 - 1 = 16895 page address
  const size_t kFirstPageAddressInSecondBlock = 16895;

  // dont execute writePage() lightly, as there are limited amount
  // of write operations to a single page.
  nand.writePage(&pageToWrite[0], kFirstPageAddressInSecondBlock);
  nand.waitUntilReady();
  nand.readPage(16895, &obtainedPage[0]);

  // page no longer writtable after first write page because it is no longer in
  // "erased" state, which is on by default.
}

void loop() {
  Serial.println("Obtained page from NAND read page operation: ");
  for (size_t i = 0; i < 2112; ++i) {
    Serial.print(obtainedPage[i]);
  }
  Serial.println();

  delay(1000);
}
