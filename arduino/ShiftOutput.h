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

#ifndef UVM_SHIFTOUTPUT_H
#define UVM_SHIFTOUTPUT_H

#include <stdint.h>

class ShiftOutput {
    uint8_t enablePin;
    uint8_t clkPin;
    uint8_t rClkPin;
    uint8_t dataPin;
  public:
    ShiftOutput(uint8_t enablePin, uint8_t clkPin, uint8_t rClkPin, uint8_t dataPin);
    void enable();
    void disable();
    inline void output(uint8_t data) { 
      output(data, false); 
    }
    inline void output16(uint16_t data) { 
      output16(data, false); 
    }
    void output(uint8_t data, bool rclk);
    void output16(uint16_t data, bool rclk);
    void clockOutput();
};

#endif //UVM_SHIFTOUTPUT_H
