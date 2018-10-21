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

VM vm;
Console console(&vm);

void setup() {
  vm_reset(&vm);
  vm.error = vm_print_error;
  vm.syscall = vm_syscall;
  Serial.begin(1200);
  console.setSerial(&Serial);
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
      Serial.println("Halted. Unknown instruction. (this should never happen)");
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

