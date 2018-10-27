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

#ifndef UVM_SHIFTINPUT_H
#define UVM_SHIFTINPUT_H

#include <stdint.h>

class ShiftInput {
    uint8_t *input;
    uint8_t chipCount;
    uint8_t loadPin;
    uint8_t clockPin;
    uint8_t clockInhPin;
    uint8_t dataPin;
  public:
    ShiftInput(uint8_t chipCount, uint8_t loadPin, uint8_t clockPin, uint8_t clockInhPin, uint8_t dataPin);
    uint8_t getChipCount();
    uint8_t *getInput();
    bool updateInput();
};

#endif //UVM_SHIFTINPUT_H
