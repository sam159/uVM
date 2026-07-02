#!/bin/bash
set -e

echo "Building assembler..."
cmake --build cmake-build-debug

echo "Testing example programs..."

PASS=0
FAIL=0

for asm_file in programs/*.asm programs/*/*.asm; do
    if [ -f "$asm_file" ]; then
        echo -n "  Testing: $asm_file ... "

        # Run assembler and capture exit code
        set +e
        output=$(./cmake-build-debug/uVM -c "$asm_file" -o /tmp/test_output.bin 2>&1)
        exit_code=$?
        set -e

        # Check for crash (signal exit)
        if [ $exit_code -gt 128 ]; then
            echo "CRASH (signal $((exit_code - 128)))"
            FAIL=$((FAIL + 1))
        elif echo "$output" | grep -qi "error:"; then
            echo "FAIL (assembly error)"
            FAIL=$((FAIL + 1))
        else
            # Execute the assembled binary and validate output
            set +e
            run_output=$(./cmake-build-debug/uVM /tmp/test_output.bin 2>&1)
            run_exit=$?
            set -e

            if echo "$run_output" | grep -q "Halted with code 0"; then
                echo "PASS"
                PASS=$((PASS + 1))
            else
                echo "FAIL (execution: exit=$run_exit)"
                FAIL=$((FAIL + 1))
            fi
        fi
    fi
done

echo ""
echo "Summary:"
echo "  Passed: $PASS"
echo "  Failed: $FAIL"
if [ $FAIL -gt 0 ]; then
    echo "FAIL: $FAIL program(s) failed"
    exit 1
fi

echo "PASS: No failures detected"
