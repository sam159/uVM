cmake_minimum_required(VERSION 3.12)
project(uVM C)

set(CMAKE_C_STANDARD 11)

add_executable(uVM src/main.c src/vm.c src/vm.h src/utils.c src/utils.h src/asm/asm.c src/asm/asm.h src/asm/token.c src/asm/token.h src/asm/tree.c src/asm/tree.h src/mm/mm.c src/mm/mm.h src/devices/memory.c src/devices/memory.h src/devices/timer.c src/devices/timer.h)
target_include_directories(uVM PRIVATE src/asm src/mm src/devices src)

# Test infrastructure
set(ASM_SOURCES src/asm/tree.c src/asm/token.c)

add_executable(test_alias tests/test_alias.c ${ASM_SOURCES})
target_include_directories(test_alias PRIVATE src/asm)

add_executable(test_data tests/test_data.c ${ASM_SOURCES})
target_include_directories(test_data PRIVATE src/asm)

add_executable(test_empty tests/test_empty.c ${ASM_SOURCES})
target_include_directories(test_empty PRIVATE src/asm)

add_executable(test_eof tests/test_eof.c ${ASM_SOURCES})
target_include_directories(test_eof PRIVATE src/asm)

add_executable(test_free tests/test_free.c ${ASM_SOURCES})
target_include_directories(test_free PRIVATE src/asm)

add_executable(test_instruction tests/test_instruction.c ${ASM_SOURCES})
target_include_directories(test_instruction PRIVATE src/asm)

add_executable(test_operand_reg tests/test_operand_reg.c ${ASM_SOURCES})
target_include_directories(test_operand_reg PRIVATE src/asm)

add_executable(test_operand_value tests/test_operand_value.c ${ASM_SOURCES})
target_include_directories(test_operand_value PRIVATE src/asm)

add_executable(test_origin tests/test_origin.c ${ASM_SOURCES})
target_include_directories(test_origin PRIVATE src/asm)

add_executable(test_operand_rel tests/test_operand_rel.c ${ASM_SOURCES})
target_include_directories(test_operand_rel PRIVATE src/asm)

add_executable(test_operand_mem tests/test_operand_mem.c ${ASM_SOURCES})
target_include_directories(test_operand_mem PRIVATE src/asm)

add_executable(test_operand_test tests/test_operand_test.c ${ASM_SOURCES})
target_include_directories(test_operand_test PRIVATE src/asm)

add_executable(test_instructions_q tests/test_instructions_q.c ${ASM_SOURCES})
target_include_directories(test_instructions_q PRIVATE src/asm)

add_executable(test_instructions_s tests/test_instructions_s.c ${ASM_SOURCES})
target_include_directories(test_instructions_s PRIVATE src/asm)

add_executable(test_instructions_t tests/test_instructions_t.c ${ASM_SOURCES})
target_include_directories(test_instructions_t PRIVATE src/asm)

add_executable(test_instructions_v tests/test_instructions_v.c ${ASM_SOURCES})
target_include_directories(test_instructions_v PRIVATE src/asm)

add_executable(test_dx tests/test_dx.c ${ASM_SOURCES})
target_include_directories(test_dx PRIVATE src/asm)

add_executable(test_resolve tests/test_resolve.c ${ASM_SOURCES})
target_include_directories(test_resolve PRIVATE src/asm)

add_executable(test_emit tests/test_emit.c ${ASM_SOURCES})
target_include_directories(test_emit PRIVATE src/asm)

add_executable(test_dw_label tests/test_dw_label.c ${ASM_SOURCES})
target_include_directories(test_dw_label PRIVATE src/asm)

add_executable(test_dw_resolve tests/test_dw_resolve.c ${ASM_SOURCES})
target_include_directories(test_dw_resolve PRIVATE src/asm)

add_executable(test_hlt_q tests/test_hlt_q.c src/vm.c)
target_include_directories(test_hlt_q PRIVATE src)
add_test(NAME test_hlt_q COMMAND test_hlt_q)

add_executable(test_memory_device tests/test_memory_device.c src/devices/memory.c src/mm/mm.c)
target_include_directories(test_memory_device PRIVATE src/mm src/devices)
add_test(NAME test_memory_device COMMAND test_memory_device)

add_executable(test_mm tests/test_mm.c src/mm/mm.c)
target_include_directories(test_mm PRIVATE src/mm)
add_test(NAME test_mm COMMAND test_mm)

add_executable(test_timer_device tests/test_timer_device.c src/devices/timer.c src/mm/mm.c)
target_include_directories(test_timer_device PRIVATE src/mm src/devices)
add_test(NAME test_timer_device COMMAND test_timer_device)
