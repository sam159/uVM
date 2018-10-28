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

#ifndef UVM_CONSOLE_H
#define UVM_CONSOLE_H

#include <HardwareSerial.h>
#include "VM.h"

enum ConsoleState_t {
  CONSOLE_NONE,
  CONSOLE_OFF,
  CONSOLE_ACTIVATE,
  CONSOLE_ACTIVE,
  CONSOLE_RUN,
  CONSOLE_STEP,
  CONSOLE_EXAMINE,
  CONSOLE_VIEW,
  CONSOLE_RESET,
  CONSOLE_CLEAR,
};
typedef enum ConsoleState_t ConsoleState;

class Console {
    ConsoleState state;
    VM *vm;
    String *inputBuffer;
    HardwareSerial *serial;
    
    int hexToDec(String);
    void printRegisters(bool);
    void printMemory(uint16_t, uint16_t);
    
    void stateActive();
    void stateView();
    void stateExamine();
    
  public:
    Console (VM*);
    void setSerial(HardwareSerial*);
    void loop();
};

#endif //UVM_CONSOLE_H
