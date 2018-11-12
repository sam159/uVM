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
 * Hello World: 3118 3302 1210 c032 3401 4114 1210 3416 e420 3406 d400 0000 48656c6c6f20576f726c640a
 * 32-bit Counter: 3601 3704 4556 5440 5330 5220 5110 D700
 */

//Options
//#define TEST_DESTRUCTIVE

#include "VM.h"
#include "Console.h"
#include "Memory.h"
#include "ShiftInput.h"
#include "ShiftOutput.h"
#include "Interface.h"
#include "pins.h"
#include "util.h"

//Expecting a Microchip 23LCV512 connected over MOSI/MISO/MCLK
Memory mem(PIN_MEMORY_SEL);
MemoryCache ICache(64, mem);
MemoryCache DCache(32, mem);

VM vm;
Console console(&vm);
ShiftInput inputs(3, PIN_INP_LOAD, PIN_INP_CLK, 0, PIN_INP_SER);
ShiftOutput disp(PIN_DISP_EN, PIN_DISP_CLK, PIN_DISP_RCLK, PIN_DISP_OUT);
Interface io(&vm, &mem, &inputs, &disp, PIN_SW_REG);

void setup() {
  //Setup VM
  vm_reset(&vm);
  vm.error = vm_print_error;
  vm.syscall = vm_syscall;
  vm.readAddr = vm_read_addr;
  vm.writeAddr = vm_write_addr;

  //Display
  for(uint8_t i = 0; i < 16; i++) {
    disp.output((uint8_t)0, true);
  }
  disp.clockOutput();
  disp.enable();

  //Interface
  io.init();

  //Disable input inhibit lines (Layout issue on Rev.A board)
  pinMode(PIN_INP_ADDR_INH, OUTPUT);
  pinMode(PIN_INP_SW_INH, OUTPUT);
  digitalWrite(PIN_INP_ADDR_INH, LOW);
  digitalWrite(PIN_INP_SW_INH, LOW);

  //Start serial
  Serial.begin(9600);
  console.setSerial(&Serial);

  //Speed control
  pinMode(PIN_SPEED, INPUT);

  //Init memory
  mem.setSize(VM_MEM_SIZE);
  if (!mem.init()) {
    Serial.println("Memory failed to init!");
    Serial.print("Tried to set MODE_SEQ. Current mode is ");
    switch(mem.readMode()) {
      case MODE_BYTE:
        Serial.println("MODE_BYTE");
        break;
      case MODE_PAGE:
        Serial.println("MODE_PAGE");
        break;
      case MODE_SEQ:
        Serial.println("MODE_SEQ");
        break;
      default:
        Serial.println(mem.readMode());
        break;
    }
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

unsigned long int nextRun = 0;
uint8_t err = 0;

void loop() {
  if (vm.run) {
    if (nextRun == 0 || nextRun < millis()) {
      vm_step(&vm);
      int delayTime = 1024 - analogRead(PIN_SPEED);
      if (delayTime < 10) {
        nextRun = 0;
      } else {
        nextRun = millis() + delayTime;
      }
    }
  } else {
    nextRun = 0;
  }
  
  console.loop();
  io.loop();
}

void vm_print_error(uint8_t err) {
  vm.R[0] = err;
  Serial.print("\nPC 0x");
  Serial.print(vm.PC, HEX);
  switch(err) {
    case VM_ERR_MISALIGN:
      Serial.println("\nHalted. PC misaligned.");
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

