#!/bin/bash
set -e

echo "Building assembler..."
cmake --build cmake-build-debug

echo "Testing example programs..."

PASS=0
FAIL=0
ERRORS=0

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
            # Parse error is expected for now (operand parsing not implemented)
            echo "PARSE ERROR (expected)"
            ERRORS=$((ERRORS + 1))
        else
            echo "PASS"
            PASS=$((PASS + 1))
        fi
    fi
done

echo ""
echo "Summary:"
echo "  Passed: $PASS"
echo "  Parse errors: $ERRORS (expected - operand parsing not yet implemented)"
echo "  Crashes: $FAIL"

if [ $FAIL -gt 0 ]; then
    echo "FAIL: $FAIL program(s) crashed"
    exit 1
fi

echo "PASS: No crashes detected"
