/*
Copyright 2018 Sam Stevens

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

#ifndef UVM_MEMORY_H
#define UVM_MEMORY_H

#include <SPI.h>

enum MemoryMode_t {
  MODE_BYTE = 0x00,
  MODE_PAGE = 0x80,
  MODE_SEQ  = 0x40
};
typedef enum MemoryMode_t MemoryMode;

enum MemoryInstruction_t {
  INSTR_READ  = 0x03,
  INSTR_WRITE = 0x02,
  INSTR_EDIO  = 0x3B,
  INSTR_RESIO = 0xFF,
  INSTR_RDMR  = 0x05,
  INSTR_WRMR  = 0x01
};
typedef enum MemoryInstruction_t MemoryInstruction;

class Memory {
    SPISettings setting;
    uint8_t csPin;
    uint16_t memSize;
    void start();
    void end();
  public:
    Memory(uint8_t csPin);
    uint16_t getSize();
    void setSize(uint16_t memSize);
    void setMode(MemoryMode mode);
    MemoryMode readMode();
    bool init();
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
    void writeRange(uint8_t data, uint16_t from, uint16_t to) ;
    bool verifyRange(uint8_t data, uint16_t from, uint16_t to);
    bool test();
};

struct MemoryCacheItem_t {
  uint16_t addr;
  uint8_t data;
  uint8_t ttl;
  bool valid;
};
typedef struct MemoryCacheItem_t MemoryCacheItem;

class MemoryCache {
    MemoryCacheItem *cache = NULL;
    Memory *memory = NULL;
    uint8_t cacheElements = 0;
  public:
    MemoryCache(uint8_t cacheElements, Memory& memory);
    uint8_t getSize();
    MemoryCacheItem* getItem(uint8_t index);
    uint8_t read(uint16_t addr);
    void update(uint16_t addr, uint8_t data);
    void clear();
};

#endif //UVM_MEMORY_H
