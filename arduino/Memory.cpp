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

#include "Memory.h"

Memory::Memory(uint8_t csPin) {
  //Set cs pin high
  this->csPin = csPin;
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);
  //Default to 64k ram
  this->memSize = 0xFFFF;
  //Store spi setting
  this->setting = SPISettings(20000000, MSBFIRST, SPI_MODE0);
}

void Memory::start() {
  digitalWrite(this->csPin, LOW);
  SPI.beginTransaction(this->setting);
}
void Memory::end() {
  SPI.endTransaction();
  digitalWrite(this->csPin, HIGH);
}

bool Memory::init() {
  //Set mode to sequential access (should be default mode)
  this->start();
  SPI.transfer(INSTR_WRMR);
  SPI.transfer(MODE_SEQ);
  this->end();

  //Verify that it was set
  this->start();
  SPI.transfer(INSTR_RDMR);
  uint8_t mode = SPI.transfer(0);
  this->end();
  
  return mode == MODE_SEQ;
}

uint16_t Memory::getSize() {
  return this->memSize;
}

void Memory::setSize(uint16_t memSize) {
  this->memSize = memSize;
}

uint8_t Memory::read(uint16_t addr) {
  this->start();
  SPI.transfer(INSTR_READ);
  SPI.transfer16(addr);
  uint8_t data = SPI.transfer(0);
  this->end();
  return data;
}

void Memory::write(uint16_t addr, uint8_t data) {
  this->start();
  SPI.transfer(INSTR_WRITE);
  SPI.transfer16(addr);
  SPI.transfer(data);
  this->end();
}

void Memory::writeRange(uint8_t data, uint16_t from, uint16_t to) {
  if (from > to) {
    return;
  }
  this->start();
  SPI.transfer(INSTR_WRITE);
  SPI.transfer16(from);
  for(uint32_t i = from; i < to; i++) {
    SPI.transfer(data);
  }
  this->end();
}

bool Memory::verifyRange(uint8_t data, uint16_t from, uint16_t to) {
  if (from > to) {
    return false;
  }
  this->start();
  SPI.transfer(INSTR_READ);
  SPI.transfer16(from);
  bool ok = true;
  for(uint32_t i = from; i < to; i++) {
    if (SPI.transfer(0) != data) {
      ok = false;
      break;
    }
  }
  this->end();
  return ok;
}

bool Memory::test() {
  //write 0x55 (01010101)
  this->writeRange(0x55, 0, this->memSize);
  //verify
  if (!this->verifyRange(0x55, 0, this->memSize)) {
    return false;
  }
  
  //write zeros
  this->writeRange(0, 0, this->memSize);
  //verify zeros
  if (!this->verifyRange(0, 0, this->memSize)) {
    return false;
  }
  return true;
}


uint8_t MemoryCache::getSize() {
  return this->cacheElements;
}
MemoryCacheItem* MemoryCache::getItem(uint8_t index) {
  if (index < this->cacheElements) {
    return &this->cache[index];
  }
  return NULL;
}

MemoryCache::MemoryCache(uint8_t cacheElements, Memory& memory) {
  this->memory = &memory;
  this->cache = (MemoryCacheItem*)calloc(cacheElements, sizeof(MemoryCacheItem));
  this->cacheElements = cacheElements;
}
uint8_t MemoryCache::read(uint16_t addr) {
  bool found = false;
  uint8_t data = 0;
  //Find in cache
  for(uint8_t i = 0; i < this->cacheElements; i++) {
    if (this->cache[i].valid) {
      if (this->cache[i].addr == addr) {
        if (this->cache[i].ttl < UINT8_MAX - 1) {
          this->cache[i].ttl = UINT8_MAX;
        }
        data = this->cache[i].data;
        found = true;
      }
    }
  }
  if (found) {
    return data;
  }
  //Not found, read from memory and add to cache
  data = this->memory->read(addr);

  //Decrement TTLs
  for(uint8_t i = 0; i < this->cacheElements; i++) {
    if (this->cache[i].ttl > 0) {
      this->cache[i].ttl--;
    }
  }

  uint8_t foundTTL = UINT8_MAX;
  uint8_t cacheSlot = 0;
  for(uint8_t i = 0; i < this->cacheElements; i++) {
    if (!this->cache[i].valid) {
      cacheSlot = i;
      break;
    }
    if (this->cache[i].ttl < foundTTL) {
      cacheSlot = i;
    }
  }
  
  this->cache[cacheSlot].addr = addr;
  this->cache[cacheSlot].data = data;
  this->cache[cacheSlot].ttl = UINT8_MAX;
  this->cache[cacheSlot].valid = true;
  
  return data;
}
void MemoryCache::update(uint16_t addr, uint8_t data) {
  for(uint8_t i = 0; i < this->cacheElements; i++) {
    if (this->cache[i].valid && this->cache[i].addr == addr) {
      this->cache[i].data = data;
      break;
    }
  }
}

