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
#include <HardwareSerial.h>
#include "console.h"

Console::Console(VM* _vm) {
  this->state = CONSOLE_NONE;
  this->vm = _vm;
  this->inputBuffer = new String("");
}

void Console::loop(HardwareSerial * serial) {
  int i;
  char d;
      
  switch(this->state) {
    case CONSOLE_NONE:
      serial->println("Press Enter to activate console.");
      this->state = CONSOLE_OFF;
      break;
    case CONSOLE_OFF:
      if(serial->available()) {
        if (serial->read() == 10) {
          serial->println("Console active. Type ? for help.");
          this->state = CONSOLE_ACTIVATE;
        }
      }
      break;
    case CONSOLE_ACTIVATE:
      serial->print("> ");
      this->state = CONSOLE_ACTIVE;
    break;
    case CONSOLE_ACTIVE:
      if(serial->available()) {
        while((d = (char)serial->read()) != -1) {
          if (d == 27) {
            //escape, deactivate console
            this->inputBuffer = new String("");
            serial->println("Console deactivated.");
            this->state = CONSOLE_NONE;
          } else if (d == '\n') {
            //Finished input
            serial->print("\n");
            if (this->inputBuffer->equals("?")) {
              serial->print(F(
"Commands\n"
"?   | this help\n"
"q   | deactivate console\n"
"r   | run until halted\n"
"s   | step one instruction\n"
"e   | examine and update memory\n"
"v   | show vm status\n"
"rst | reset PC and registers\n"
"clr | reset and clear memory\n"
              ));
              serial->print("> ");
            } else if (this->inputBuffer->equals("q")) {
              serial->println("Console deactivated.");
              this->state = CONSOLE_NONE;
            } else if (this->inputBuffer->equals("r")) {
              this->state = CONSOLE_RUN;
            } else if (this->inputBuffer->equals("s")) {
              this->state = CONSOLE_STEP;
            } else if (this->inputBuffer->equals("e")) {
              this->state = CONSOLE_EXAMINE;
            } else if (this->inputBuffer->equals("v")) {
              this->state = CONSOLE_VIEW;
            } else if (this->inputBuffer->equals("rst")) {
              this->state = CONSOLE_RESET;
            } else if (this->inputBuffer->equals("clr")) {
              this->state = CONSOLE_CLEAR;
            } else {
              serial->println("Unknown command. Type ? for help.");
              serial->print("> ");
            }
            this->inputBuffer = new String("");
          } else if (isAlphaNumeric(d) || isPunct(d) || isSpace(d)) {
            //Append input to buffer
            this->inputBuffer->concat(d);
            serial->print(d);
          }
        }
      }
      break;
    case CONSOLE_RUN:
      break;
    case CONSOLE_STEP:
      break;
    case CONSOLE_EXAMINE:
      break;
    case CONSOLE_VIEW:
      //PC
      serial->print(  "PC   ");
      serial->print(this->vm->PC, HEX);
      serial->print("\n");
      
      //Registers
      serial->print(  "R    ");
      for(i = 0; i < VM_REG_SIZE; i++) {
        if (this->vm->R[i] < 0x10) {
          serial->print("0");
        }
        serial->print(this->vm->R[i], HEX);
        serial->print(" ");
      }
      serial->print("\n");

      //Memory
      serial->println("      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
      for(i = 0; i < VM_MEM_SIZE; i++) {
        if (i % 16 == 0) {
          if (i > 0) {
            serial->print("\n");
            if (i % 128 == 0 ) {
              serial->println("      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
            }
          }
          serial->print("0x");
          if (i < 0x10) {
            serial->print("0");
          }
          serial->print(i, HEX);
          serial->print(" ");
        }
        uint8_t m = this->vm->M[i];
        if (m < 0x10) {
          serial->print("0");
        }
        serial->print(m, HEX);
        serial->print(" ");
      }
      serial->print("\n");

      this->state = CONSOLE_ACTIVATE;
      break;
    case CONSOLE_RESET:
      vm_reset(this->vm);
      serial->println("VM Reset");
      this->state = CONSOLE_ACTIVATE;
      break;
    case CONSOLE_CLEAR:
      vm_reset(this->vm);
      for(i = 0; i < VM_MEM_SIZE; i++) {
        this->vm->M[i] = 0;
      }
      serial->println("VM Reset & Memory Cleared");
      this->state = CONSOLE_ACTIVATE;
      break;
  }
}

