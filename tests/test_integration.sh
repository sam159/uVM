#!/bin/bash
set -e

echo "Building assembler..."
cmake --build cmake-build-debug -- -j4

PASS=0
FAIL=0

echo "Running unit tests..."

for test_bin in cmake-build-debug/test_*; do
    if [ -x "$test_bin" ]; then
        test_name=$(basename "$test_bin")
        echo -n "  Testing: $test_name ... "

        set +e
        output=$("$test_bin" 2>&1)
        run_exit=$?
        set -e

        if [ $run_exit -eq 0 ]; then
            echo "PASS"
            PASS=$((PASS + 1))
        else
            echo "FAIL (exit=$run_exit)"
            echo "$output" | sed 's/^/    /'
            FAIL=$((FAIL + 1))
        fi
    fi
done

echo ""
echo "Testing example programs..."

TMPOUTPUT=`mktemp --suffix=.bin`
trap "rm $TMPOUTPUT" exit

for asm_file in programs/*.asm programs/*/*.asm; do
    if [ -f "$asm_file" ]; then
        echo -n "  Testing: $asm_file ... "

        # Run assembler and capture exit code
        set +e
        output=$(./cmake-build-debug/uVM -c "$asm_file" -o $TMPOUTPUT 2>&1)
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
            ./cmake-build-debug/uVM $TMPOUTPUT 2>&1 >/dev/null
            run_exit=$?
            set -e

            if [ $run_exit -eq 0 ]; then
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
