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
#include <errno.h>
#include "Console.h"

Console::Console(VM* _vm) {
  this->state = CONSOLE_NONE;
  this->vm = _vm;
  this->inputBuffer = new String("");
  this->serial = NULL;
}

int Console::hexToDec(String str) {
  errno = 0;
  long int result = strtol(str.c_str(), NULL, 16);
  if (errno != 0 || result < 0) {
    return -1;
  }
  return (int)result;
}

void Console::setSerial(HardwareSerial *serial) {
  this->serial = serial;
}

void Console::printRegisters(uint8_t padding) {
  if (padding < 3) {
    padding = 3;
  }

  serial->print('R');
  for(uint8_t x = 1; x < padding; x++) serial->print(' ');
  serial->println(" 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");

  for(uint8_t x = 0; x < padding; x++) serial->print(' ');
  for(uint8_t i = 0; i < 16; i++) {
    uint8_t r = vm_get_r(vm, i);
    if (r < 0x10) {
      serial->print("0");
    }
    serial->print(r, HEX);
    serial->print(" ");
  }
  serial->print("\n");

  serial->print("RX");
  for(uint8_t x = 2; x < padding; x++) serial->print(' ');
  serial->println("                    AX   BX   CX   DX   EX   FX");
  
  serial->print(  "                  ");
  for(uint8_t x = 0; x < padding; x++) serial->print(' ');
  for(uint8_t i = 0xA; i <= 0xF; i++) {
    uint16_t rx = vm_get_rx(vm, i);
    uint8_t rxh = (rx & 0xFF00) >> 8,
            rxl = rx & 0xFF;

    if (rxh < 0x10) {
      serial->print("0");
    }
    serial->print(rxh, HEX);
    if (rxl < 0x10) {
      serial->print("0");
    }
    serial->print(rxl, HEX);
    serial->print(" ");
  }
  serial->print("\n");
}

void Console::printMemory(uint16_t from, uint16_t to) {
  uint16_t i = from;
  serial->println("      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
  if (from > 0 && from % 16 != 0) {
    i = from - (from % 16);
    serial->print("0x");
    if (i < 0x10) {
      serial->print("0");
    }
    serial->print(i, HEX);
    serial->print(" ");
    for(; i < from; i++) {
      serial->print("   ");
    }
  }
  for(i = from; i <= to && i < VM_MEM_SIZE; i++) {
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
    uint8_t m = this->vm->readAddr(i, false);
    if (m < 0x10) {
      serial->print("0");
    }
    serial->print(m, HEX);
    serial->print(" ");
  }
  serial->print("\n");
}

void Console::loop() {
  HardwareSerial * serial = this->serial;
  uint32_t i;
  
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
      this->stateActive();
      break;
    case CONSOLE_RUN:
      if(this->vm->halted == false) {
        vm_step(this->vm);
      } else {
        serial->print("VM Halted. PC=0x");
        serial->print(this->vm->PC, HEX);
        serial->println(". Registers:");
        this->printRegisters(0);
        this->state = CONSOLE_ACTIVATE;
      }
      break;
    case CONSOLE_STEP:
      if (this->vm->halted == false) {
        vm_step(this->vm);
        if (this->vm->halted == true) {
          serial->print("VM Halted. ");
        }
        serial->print("PC=0x");
        serial->print(this->vm->PC, HEX);
        serial->println(". Registers:");
        this->printRegisters(0);
      }
      this->state = CONSOLE_ACTIVATE;
      break;
    case CONSOLE_EXAMINE:
      this->stateExamine();
      break;
    case CONSOLE_VIEW:
      this->stateView();
      break;
    case CONSOLE_RESET:
      vm_reset(this->vm);
      serial->println("VM Reset");
      this->state = CONSOLE_ACTIVATE;
      break;
    case CONSOLE_CLEAR:
      vm_reset(this->vm);
      for(i = 0; i < VM_MEM_SIZE; i++) {
        this->vm->writeAddr(i, 0);
      }
      serial->println("VM Reset & Memory Cleared");
      this->state = CONSOLE_ACTIVATE;
      break;
  }
}

void Console::stateActive() {
  HardwareSerial * serial = this->serial;
  char d;
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
            "?   | This help\n"
            "q   | Deactivate console\n"
            "r   | Run until halted\n"
            "s   | Step one instruction\n"
            "e   | Examine/update\n"
            "v   | Show vm status\n"
            "rst | Reset PC and registers\n"
            "clr | Reset and clear memory\n"
          ));
          serial->print("> ");
        } else if (this->inputBuffer->equals("q")) {
          serial->println("Console deactivated.");
          this->state = CONSOLE_NONE;
        } else if (this->inputBuffer->equals("r")) {
          this->vm->halted = false;
          this->state = CONSOLE_RUN;
        } else if (this->inputBuffer->equals("s")) {
          this->vm->halted = false;
          this->state = CONSOLE_STEP;
        } else if (this->inputBuffer->equals("e")) {
          serial->println("Type q to return to console. Type ? for help.");
          serial->print("E> ");
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
    }//while serial read
  }//if serial available
}//Console::stateActive

void Console::stateView() {
  
  //PC
  serial->print("PC   ");
  if (this->vm->PC < 0x1000) {
    serial->print("0");
  }
  if (this->vm->PC < 0x100) {
    serial->print("0");
  }
  if (this->vm->PC < 0x10) {
    serial->print("0");
  }
  serial->print(this->vm->PC, HEX);
  serial->print("\n");
  
  //Registers
  this->printRegisters(5);
  serial->print("\n");

  //Memory
  this->printMemory(0, 0xFF);
  serial->println("Output limited to 00-FF");

  this->state = CONSOLE_ACTIVATE;
}//Console::stateView

void Console::stateExamine() {
  char d;
  uint16_t location, value;
  if(serial->available()) {
    while((d = (char)serial->read()) != -1) {
      if (d == 27) {
        //escape, go back to console
        this->inputBuffer = new String("");
        this->state = CONSOLE_ACTIVATE;
      } else if (d == '\n') {
        //Finished input
        serial->print("\n");
        this->inputBuffer->toUpperCase();
        if (this->inputBuffer->equals("?")) {
          serial->print(F(
            "Commands\n"
            "?       | This help\n"
            "q       | Return to console\n"
            "PC      | Show program counter\n"
            "PC=yyyy | Set program counter\n"
            "R       | Show register contents\n"
            "Rx=yy   | Update register\n"
            "Myy,zz  | Show zz bytes of memory starting at xx\n"
            "Myy=zz  | Update memory at yy with zz, may repeat zz for subsequent bytes\n"
            "x = 0-F, yy/zz = 00-FF\n"
          ));
        } else if (this->inputBuffer->equals("Q")) {
          this->state = CONSOLE_ACTIVATE;
        } else if (this->inputBuffer->equals("PC")) {
          serial->println(this->vm->PC, HEX);
        } else if (this->inputBuffer->startsWith("PC") && this->inputBuffer->indexOf("=") != -1) {
           value = this->hexToDec(this->inputBuffer->substring(this->inputBuffer->indexOf("=")+1));
           if (value >= 0 && value < VM_MEM_SIZE) {
             this->vm->PC = value;
           } else {
             serial->println("Invalid value");
           }
        } else if (this->inputBuffer->equals("R")) {
          this->printRegisters(0);
        } else if (this->inputBuffer->startsWith("R") && this->inputBuffer->indexOf("=") != -1) {
          location = this->hexToDec(this->inputBuffer->substring(1, this->inputBuffer->indexOf("=")));
          value = this->hexToDec(this->inputBuffer->substring(this->inputBuffer->indexOf("=")+1));
          if (location >= 0 && location < VM_REG_SIZE) {
            if (value >= 0 && value <= 255) {
              this->vm->R[location] = (uint8_t)value;
            } else {
              serial->println("Invalid value");
            }
          } else {
            serial->println("Invalid register");
          }
        } else if (this->inputBuffer->startsWith("M") && this->inputBuffer->indexOf(",") != -1) {
          location = this->hexToDec(this->inputBuffer->substring(1, this->inputBuffer->indexOf(",")));
          value = this->hexToDec(this->inputBuffer->substring(this->inputBuffer->indexOf(",")+1));
          if (value + location >= VM_MEM_SIZE) {
            value = VM_MEM_SIZE-1;
          }
          if (location >= 0 && location < VM_MEM_SIZE) {
            if (value > 0) {
              this->printMemory(location, location + value);
            } else {
              serial->println("Invalid value");
            }
          } else {
            serial->println("Invalid start location");
          }
        } else if (this->inputBuffer->startsWith("M") && this->inputBuffer->indexOf("=") != -1) {
          location = this->hexToDec(this->inputBuffer->substring(1, this->inputBuffer->indexOf("=")));
          String x = this->inputBuffer->substring(this->inputBuffer->indexOf("=")+1);
          if (x.length() % 2 != 0) {
            serial->println("Invalid data. 2 characters per byte required");
          } else {
            for(unsigned int i = 0; i < x.length(); i += 2) {
              value = this->hexToDec(x.substring(i, i+2));
              if (value >= 0 && value <= 255) {
                this->vm->writeAddr(location++, value);
              } else {
                serial->println(i);
                serial->println(x.substring(i, i+2));
                serial->println(value);
                serial->println("Invalid data.");
                break;
              }
            }
          }
        } else {
          serial->println("Unknown command.");
        }
        if (this->state == CONSOLE_EXAMINE) {
          serial->print("E> ");
        }
        this->inputBuffer = new String("");
      } else if (isAlphaNumeric(d) || isPunct(d) || isSpace(d)) {
        //Append input to buffer
        this->inputBuffer->concat(d);
        serial->print(d);
      }
    }//while serial read
  }//if serial available
}//Console::stateExamine

