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
#include "ShiftInput.h"

ShiftInput::ShiftInput(uint8_t chipCount, uint8_t loadPin, uint8_t clockPin, uint8_t clockInhPin, uint8_t dataPin) {
  this->input = (uint8_t*)calloc(chipCount, sizeof(uint8_t));
  this->chipCount = chipCount;
  this->loadPin = loadPin;
  this->clockPin = clockPin;
  this->clockInhPin = clockInhPin;
  this->dataPin = dataPin;
  pinMode(loadPin, OUTPUT);
  digitalWrite(loadPin, HIGH);
  pinMode(clockPin, OUTPUT);
  digitalWrite(clockPin, LOW);
  pinMode(clockInhPin, OUTPUT);
  if (clockInhPin > 0) {  
    digitalWrite(clockInhPin, HIGH);
    pinMode(dataPin, INPUT);
  }
}

uint8_t ShiftInput::getInput(uint8_t chip) {
  if (chip <= this->chipCount) {
    return this->input[chip];
  }
  return 0;
}

uint8_t ShiftInput::getChipCount() {
  return this->chipCount;
}

bool ShiftInput::updateInput() {
  //Load values
  digitalWrite(this->loadPin, LOW);
  digitalWrite(this->clockPin, HIGH);
  digitalWrite(this->loadPin, HIGH);
  digitalWrite(this->clockPin, LOW);
  
  bool change = false;
  uint8_t input = 0;
  if (this->clockInhPin > 0){
    digitalWrite(this->clockInhPin, LOW);
  }
  for(uint8_t chip = 0; chip < this->chipCount; chip++) {
    input = 0;
    for(uint8_t inputBit = 0; inputBit < 8; inputBit++) {
      input |= (digitalRead(this->dataPin) == HIGH ? 1 : 0) << inputBit;
      digitalWrite(clockPin, HIGH);
      digitalWrite(clockPin, LOW);
    }
    if (this->input[chip] != input) {
      this->input[chip] = input;
      change = true;
    }
  }
  if (this->clockInhPin > 0){
    digitalWrite(this->clockInhPin, HIGH);
  }
  
  return change;
}
