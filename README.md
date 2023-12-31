# TEIDESAT-I Radiation memory experiment

Testing the radiation resistance of a FRAM, MRAM, NAND Flash, NOR Flash and EEPROM is one of the experiments of the cubesat TEIDESAT-I; the objective is to verify that the memories are in a failureless operational state.

## Breakout board

An arduino chip is to be connected with the following board:

![docs/breaktout_board.png](docs/breakout_board.png)

## How to build

Four main files at <code>src/</code> represent each a build target for platform.io, one for each memory type. Using VSCode and PlatformIO Extension head to PlatformIO extension's interface and press build on demand when looking into executing one of those main files.

![EEPROM build example](docs/build_example_eeprom.PNG)

Upload and monitor altenatively once an arduino has been connected to the computer.

## TODO

 - Find a better way to have independent sketch main files in <code>src/</code> so that they can be compiled independently. Current approach involves excluding specific main files in the [platformio.ini](platformio.ini) depending on the build target, but if a new main file is added to <code>src/</code> while not being aware of the approach then build problems will arise.

 - Test on real hardware, right now it is all theoretical programming based on the memory datasheets. 30/8/2023

 - Because EEPROM has a delay when writing, check if the current code for <code>writeByte</code> and <code>writePage</code> is valid or if they need a delay to take into account the write cycle. Maybe reading the WIP flag on the status register is a viable approach. 30/8/2023

 - In [EEPROM's](lib/MemoryPayload/src/memory_eeprom.cpp) <code>writeByte</code> and <code>writePage</code> an enable write is performed first, but I am unsure about whether it's write enable instruction can be always performed or only if the memory is not busy. 30/8/2023

 - Implement fast read in [FRAM memory class](lib/MemoryPayload/src/memory_fram.h). 30/8/2023.
 
 - Expand [NANDFlash](lib/MemoryPayload/src/memory_nand_flash.h) class definition with read methods for dual and quad transmission modes. 5/9/2023
 
 - Update [NAND Flash](lib/MemoryPayload/src/memory_nand_flash.h)'s interface to allow buffer mode read/write. 5/9/2023
 
 - Check if SR Addresses in [NAND Flash](lib/MemoryPayload/src/memory_nand_flash.h) corresponding to SR-1, SR-2, SR-3 are  either 0, 1, 2 respectively or 1, 2, 3. 5/9/2023
 
 - Verify for [NAND Flash](lib/MemoryPayload/src/memory_nand_flash.h) that BUF = 1 after a Page Data Buffer, because
the datasheet (8.2.26) mentions that all instructions will be done in buffer mode after a Page Data Buffer instruction has been performed. 6/9/2023

 - Change [NAND Flash](lib/MemoryPayload/src/memory_nand_flash.h) <code>readByte()</code>'s behavior so that it doesn't always load the data buffer page. Possibly by allowing separate page data load. 6/9/2023
 
 - In [NAND Flash](lib/MemoryPayload/src/memory_nand_flash.h), check if <code>readByte()</code> and <code>readPage()</code> include the 64 ECC bytes on the output or can be accessed. 6/9/2023
 
 - In [NAND Flash](lib/MemoryPayload/src/memory_nand_flash.h), check if write enable and write status register instructions can be executed anytime. 6/9/2023

 - In [NAND Flash](lib/MemoryPayload/src/memory_nand_flash.h), confirm whether <code>eraseBlock</code> does indeed erase the block associated with the addressed page. 6/9/2023

 - In [NOR Flash](lib/MemoryPayload/src/memory_nor_flash.h) update the SPI clock transmission speed constant to *90 MHz* whenever the class has been updated to use Double Transfer Rate. 7/9/2023

 - Define [NOR Flash](lib/MemoryPayload/src/memory_nor_flash.h) according to it's datasheet. 12/09/2023

 - Check powerup delay time in the [FRAM's](lib/MemoryPayload/src/memory_fram.h) datasheet to update the [fram_test.cpp (fram's main file)](src/fram_test.cpp). 12/09/2023

 - Add "@post Write is disabled." comment on write functions on the memories as necessary. 12/09/2023
