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

#ifndef UVM_PINS_H
#define UVM_PINS_H

//Display / Shift register output
#define PIN_DISP_EN   8
#define PIN_DISP_CLK  16
#define PIN_DISP_RCLK 17
#define PIN_DISP_OUT  15

//Inputs
#define PIN_SPEED        14
#define PIN_SW_REG       7
#define PIN_INP_SER      2
#define PIN_INP_CLK      3
#define PIN_INP_ADDR_INH 4
#define PIN_INP_SW_INH   5
#define PIN_INP_LOAD     6

//I/O
#define PIN_IO_INT 9

//Memory
#define PIN_MEMORY_SEL 10

#endif
