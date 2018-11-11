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

#include <Arduino.h>
#include "Interface.h"
#include "util.h"

Interface::Interface(VM *vm, Memory *mem, ShiftInput *input, ShiftOutput *disp, int regSwitchPin) {
  this->vm = vm;
  this->mem = mem;
  this->input = input;
  this->disp = disp;

  //Inputs
  this->regSwitchPin = regSwitchPin;
}

void Interface::init() {
  pinMode(regSwitchPin, INPUT);
  data = mem->read(addr);

  //Lamp test
  for(uint8_t i = 0; i < 16; i++) {
    disp->output(i==0 ? 0xFF : 0x0);
    disp->clockOutput();
    delay(50);
  }
  for(uint8_t i = 0; i < 16; i++) {
    disp->output(0xFF);
  }
  disp->clockOutput();
  delay(500);
}

void Interface::loop() {

  //Process inputs

  uint8_t swLast = switchState;
  if (input->updateInput()) {
    uint8_t addrInputH = ~reverse8(input->getInput(0));
    uint8_t addrInputL = ~reverse8(input->getInput(1));
    uint8_t swInput = input->getInput(2);
    uint16_t addrInput = (addrInputH << 8) + addrInputL;
    //Check for buttons on rising edge
    if ((swLast & SW_RUN) == 0 && (swInput & SW_RUN) == SW_RUN) {
      if (vm->run) {
        vm->run = false;
      } else {
        vm->run = true;
        vm->halted = false;
      }
    }
    else if ((swLast & SW_STEP) == 0 && (swInput & SW_STEP) == SW_STEP) {
      if (vm->run == false) {
        vm->halted = false;
        vm_step(vm);
      }
    }
    else if ((swLast & SW_RESET) == 0 && (swInput & SW_RESET) == SW_RESET) {
      if (vm->run == false) {
        vm_reset(vm);
      }
    }
    else if ((swLast & SW_CLEAR) == 0 && (swInput & SW_CLEAR) == SW_CLEAR) {
      if (vm->run == false) {
        if (clearTimer == 0) {
          clearTimer = millis() + 1000;
        }
      }
    }
    else if ((swLast & SW_CLEAR) == SW_CLEAR && (swInput & SW_CLEAR) == 0) {
      clearTimer = 0;
    }
    else if ((swLast & SW_DEPOSIT) == 0 && (swInput & SW_DEPOSIT) == SW_DEPOSIT) {
      data = addrInputH;
      vm->writeAddr(addr, data);
    }
    else if ((swLast & SW_DEPOSIT_NEXT) == 0 && (swInput & SW_DEPOSIT_NEXT) == SW_DEPOSIT_NEXT) {
      data = addrInputH;
      vm->writeAddr(addr, data);
      addr++;
    }
    else if ((swLast & SW_EXAMINE) == 0 && (swInput & SW_EXAMINE) == SW_EXAMINE) {
      addr = addrInput;
      data = mem->read(addr);
    }
    else if ((swLast & SW_EXAMINE_NEXT) == 0 && (swInput & SW_EXAMINE_NEXT) == SW_EXAMINE_NEXT) {
      addr++;
      data = mem->read(addr);
    }

    switchState = swInput;
  }

  if (vm->run) {
    clearTimer = 0;
  }

  if (clearTimer > 0) {
    if (clearTimer < millis()) {
      vm_reset(vm);
      disp->output16(0xFFFF);
      for(uint8_t i = 0; i < 14; i++) {
        disp->output((uint8_t)0);
      }
      disp->clockOutput();
      mem->writeRange(0, 0, VM_MEM_SIZE);
      addr = 0;
      data = 0;
      delay(500);
      clearTimer = 0;
    }
  }

  //Update Display

  //Address
  disp->output((uint8_t)(addr & 0xFF));
  disp->output((uint8_t)(addr >> 8));

  //Run/Halt
  uint8_t status = 0;
  if (vm->run) {
    status |= 0x80;
  }
  if (vm->halted) {
    status |= 0x40;
  }
  disp->output(status);

  //Data
  disp->output(data);

  //Program counter and Instruction
  uint16_t pc = vm->PC;
  uint16_t instr = (mem->read(pc) << 8) + mem->read(pc + 1);
//  disp->output((uint8_t)0xF);
//  disp->output((uint8_t)0xF);
//  disp->output((uint8_t)0xF);
//  disp->output((uint8_t)0xF);
  disp->output16(instr);
  disp->output16(pc);

  //Registers
//  for(uint8_t i = 0; i < 8; i++) {
//    disp->output((uint8_t)0);
//  }
  short int reg;
  if (digitalRead(regSwitchPin) == HIGH) {
    //Output 8-F
     for(reg=0xF; reg >= 0x8; reg--) {
      disp->output(vm->R[reg]);
    }
  } else {
    //Output 0-F
     for(reg=0x7; reg >= 0; reg--) {
      disp->output(vm->R[reg]);
    }
  }
  disp->clockOutput();
}

