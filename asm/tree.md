# Syntax Tree

## Types

ASMProgram
- filename
- lines
- line count

ASMProgramLine
- address – uint16_t
- line number – uint16_t
- type – ASMProgramLineType
- union of
  - ASMProgramOrigin
  - ASMProgramData
  - ASMProgramAlias
  - ASMProgramInstruction
  - ASMProgramEmpty
- comment

ASMProgramLineType enum
- origin
- data
- alias
- instruction
- empty

ASMProgramOrigin
- address – uint16_t

ASMProgramData
- data type – ASMProgramDataType
- data
- data length

ASMProgramDataType enum
- DB
- DW
- DX
- STZ
- STL

ASMProgramAlias
- variable
- value

ASMProgramInstruction
- label
- op – ASMProgramInstructionType
- operands – ASMProgramInstructionOperand
- operand count

ASMProgramInstructionType enum
- every vm instruction type

ASMProgramInstructionOperand
- type – ASMProgramInstructionOperandType
- union of
  - value16 - uint16_t
  - value8 - uint8_t
  - value4 - uint8_t
  - variable - string
  - rel - int16_t
  - reg8 - uint8_t (reg8 double)
  - reg16 - uint16_t
  - reg16 rel
    - register - uint8_t
    - offset - int8_t
  - label - string (including label high, label low)
  - test
    - type - ASMProgramInstructionOperandTestType
    - negated - bool

ASMProgramInstructionOperandType enum
- value16
- value8
- value4
- rel
- reg8
- reg8 double
- reg16
- reg16 rel
- variable
- label high
- label low
- label
- test

ASMProgramInstructionOperandTestType enum
- EQ
- LT
- LTE
- GT
- GTE