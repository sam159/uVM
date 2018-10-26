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

/**
 * Example Programs
 * Hello World: 0x311833021210c0323401411412103416e4203406d400000048656c6c6f20576f726c640a
 */

#include "vm.h"
#include "console.h"
#include "memory.h"

VM vm;
Console console(&vm);

//Expecting a Microchip 23LCV512 connected over MOSI/MISO/MCLK
Memory mem(20000000, MSBFIRST, SPI_MODE0, 4);
MemoryCache ICache(64, mem);
MemoryCache DCache(32, mem);

void setup() {
  //Setup VM
  vm_reset(&vm);
  vm.error = vm_print_error;
  vm.syscall = vm_syscall;
  vm.readAddr = vm_read_addr;
  vm.writeAddr = vm_write_addr;

  //Start serial
  Serial.begin(9600);
  console.setSerial(&Serial);

  //Init memory
  SPI.begin();
  mem.setSize(VM_MEM_SIZE);
  if (!mem.init()) {
    Serial.println("Memory failed to init!");
  } else {
#ifdef TEST_DESTRUCTIVE
    Serial.print("Testing ");
    Serial.print(mem.getSize(), DEC);
    Serial.print(" bytes memory... ");
    unsigned long startTime = millis();
    if (mem.test()) {
      Serial.println("OK");
    } else {
      Serial.println("ERROR!");
    }
    Serial.print("Tested in ");
    Serial.print(millis() - startTime, DEC);
    Serial.println("ms");
#endif
  }
}

void loop() {
  console.loop();
}

void vm_print_error(uint8_t err) {
  Serial.print("\n");
  switch(err) {
    case VM_ERR_MISALIGN:
      Serial.println("Halted. PC misaligned.");
      break;
    case VM_ERR_UNKNOWN_OP:
      //Will only happen if an instruction is not handled in the vm
      Serial.println("Halted. Unknown instruction.");
      break;
    case VM_ERR_OUT_OF_BOUNDS:
      Serial.println("Halted. Operation out of bounds.");
      break;
    default:
      Serial.println("Halted. Unknown Error.");
      break;
  }
}

uint8_t vm_syscall(VM* vm, uint8_t callno, uint8_t imm) {
//  Serial.print("SYSCALL 0x");
//  Serial.print(callno, HEX);
//  Serial.print(". IMM=");
//  Serial.print(imm);
//  Serial.println("");
  switch(callno) {
    case 0:
      delay(1<<imm);
      break;
    case 1:
      delayMicroseconds(1<<imm);
      break;
    case 2:
      if (imm < VM_REG_SIZE) {
        Serial.print((char)vm->R[imm]);
      }
      break;
    default:
      return 0;
  }
  return 0;
}

uint8_t vm_read_addr(uint16_t addr, bool instruction) {
  if (instruction) {
    return ICache.read(addr);
  } else {
    return DCache.read(addr);
  }
}

void vm_write_addr(uint16_t addr, uint8_t data) {
  mem.write(addr, data);
  ICache.update(addr, data);
  DCache.update(addr, data);
}

void print_cache_state(MemoryCache& cache) {
  for(uint8_t i = 0; i < cache.getSize(); i++) {
    MemoryCacheItem* item = cache.getItem(i);
    
    Serial.print(i);
    Serial.print(": ");
    if (item->valid) {
      Serial.print(item->addr, HEX);
      Serial.print("=");
      Serial.print(item->data, HEX);
      Serial.print(" TTL ");
      Serial.print(item->ttl);
      Serial.println("");
    } else {
      Serial.println("Nil");
    }
  }
}

