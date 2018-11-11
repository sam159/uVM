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
#include "ShiftOutput.h"

ShiftOutput::ShiftOutput(uint8_t enablePin, uint8_t clkPin, uint8_t rClkPin, uint8_t dataPin) {
  this->enablePin = enablePin;
  this->clkPin = clkPin;
  this->rClkPin = rClkPin;
  this->dataPin = dataPin;

  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);
  pinMode(clkPin, OUTPUT);
  digitalWrite(clkPin, LOW);
  pinMode(rClkPin, OUTPUT);
  digitalWrite(rClkPin, LOW);
  pinMode(dataPin, OUTPUT);
  digitalWrite(dataPin, LOW);
}

void ShiftOutput::enable() {
  digitalWrite(this->enablePin, LOW);
}
void ShiftOutput::disable() {
  digitalWrite(this->enablePin, HIGH);
}
void ShiftOutput::output(uint8_t data, bool clockOutput) {
  shiftOut(this->dataPin, this->clkPin, LSBFIRST, data);
  if (clockOutput) {
    this->clockOutput();
  }
}
void ShiftOutput::output16(uint16_t data, bool clockOutput) {
  shiftOut(this->dataPin, this->clkPin, LSBFIRST, data & 0xFF);
  shiftOut(this->dataPin, this->clkPin, LSBFIRST, (data >> 8));
  if (clockOutput) {
    this->clockOutput();
  }
}
void ShiftOutput::clockOutput() {
  digitalWrite(this->rClkPin, HIGH);
  digitalWrite(this->rClkPin, LOW);
}

