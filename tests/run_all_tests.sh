#!/bin/bash
# run_all_tests.sh — compile and run all tests, report PASS/FAIL

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
CC=gcc
CFLAGS="-fPIC -fsanitize=address -g"
ANIMATE_OBJ="$ROOT_DIR/animate.o"

PASS=0
FAIL=0

for src in "$SCRIPT_DIR"/test_*.c; do
    name="$(basename "$src" .c)"
    bin="$SCRIPT_DIR/$name"

    # Compile
    if ! $CC $CFLAGS -I"$ROOT_DIR" "$src" "$ANIMATE_OBJ" -o "$bin" 2>/tmp/"$name"_compile.log; then
        echo "FAIL (compile) $name"
        cat /tmp/"$name"_compile.log
        FAIL=$((FAIL + 1))
        continue
    fi

    # Run
    if "$bin" > /tmp/"$name"_run.log 2>&1; then
        echo "PASS $name"
        PASS=$((PASS + 1))
    else
        echo "FAIL (runtime) $name"
        cat /tmp/"$name"_run.log
        FAIL=$((FAIL + 1))
    fi

    rm -f "$bin"
done

echo ""
echo "Results: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
