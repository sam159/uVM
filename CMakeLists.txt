cmake_minimum_required(VERSION 3.12)
project(uVM C)

set(CMAKE_C_STANDARD 11)

add_executable(uVM main.c vm.c vm.h asm/asm.c asm/asm.h asm/token.c asm/token.h asm/tree.c asm/tree.h)
target_include_directories(uVM PRIVATE asm)

# Test infrastructure
set(ASM_SOURCES asm/tree.c asm/token.c)

add_executable(test_alias tests/test_alias.c ${ASM_SOURCES})
target_include_directories(test_alias PRIVATE asm)

add_executable(test_data tests/test_data.c ${ASM_SOURCES})
target_include_directories(test_data PRIVATE asm)

add_executable(test_empty tests/test_empty.c ${ASM_SOURCES})
target_include_directories(test_empty PRIVATE asm)

add_executable(test_eof tests/test_eof.c ${ASM_SOURCES})
target_include_directories(test_eof PRIVATE asm)

add_executable(test_free tests/test_free.c ${ASM_SOURCES})
target_include_directories(test_free PRIVATE asm)

add_executable(test_instruction tests/test_instruction.c ${ASM_SOURCES})
target_include_directories(test_instruction PRIVATE asm)

add_executable(test_operand_reg tests/test_operand_reg.c ${ASM_SOURCES})
target_include_directories(test_operand_reg PRIVATE asm)

add_executable(test_operand_value tests/test_operand_value.c ${ASM_SOURCES})
target_include_directories(test_operand_value PRIVATE asm)

add_executable(test_origin tests/test_origin.c ${ASM_SOURCES})
target_include_directories(test_origin PRIVATE asm)

add_executable(test_operand_rel tests/test_operand_rel.c ${ASM_SOURCES})
target_include_directories(test_operand_rel PRIVATE asm)

add_executable(test_operand_mem tests/test_operand_mem.c ${ASM_SOURCES})
target_include_directories(test_operand_mem PRIVATE asm)

add_executable(test_operand_test tests/test_operand_test.c ${ASM_SOURCES})
target_include_directories(test_operand_test PRIVATE asm)

add_executable(test_instructions_q tests/test_instructions_q.c ${ASM_SOURCES})
target_include_directories(test_instructions_q PRIVATE asm)

add_executable(test_instructions_s tests/test_instructions_s.c ${ASM_SOURCES})
target_include_directories(test_instructions_s PRIVATE asm)

add_executable(test_instructions_t tests/test_instructions_t.c ${ASM_SOURCES})
target_include_directories(test_instructions_t PRIVATE asm)
