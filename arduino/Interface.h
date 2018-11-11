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

#ifndef UVM_INTERFACE_H
#define UVM_INTERFACE_H

#define SW_RUN          0x40
#define SW_STEP         0x80
#define SW_RESET        0x10
#define SW_CLEAR        0x20
#define SW_DEPOSIT      0x02
#define SW_DEPOSIT_NEXT 0x01
#define SW_EXAMINE      0x08
#define SW_EXAMINE_NEXT 0x04

#include <stdint.h>
#include "VM.h"
#include "ShiftInput.h"
#include "ShiftOutput.h"
#include "Memory.h"

class Interface {
    VM *vm;
    Memory *mem;
    ShiftInput *input;
    ShiftOutput *disp;
    int regSwitchPin = 0;
    uint8_t switchState = 0;
    uint16_t addr = 0;
    uint8_t data = 0;
    unsigned long int clearTimer = 0;
  public:
    Interface(VM *vm, Memory *mem, ShiftInput *input, ShiftOutput *disp, int regSwitchPin);
    void init();
    void loop();
};

#endif
