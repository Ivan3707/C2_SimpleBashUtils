#!/usr/bin/env bash

set -u

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
GREP="$ROOT_DIR/src/grep/s21_grep"

TMP_DIR="$ROOT_DIR/tests/.tmp_grep"

PASS=0
FAIL=0

cleanup() {
    rm -rf "$TMP_DIR"
}

trap cleanup EXIT

mkdir -p "$TMP_DIR"

normalize() {
    tr -d '\r' |
        sed -E 's#([A-Za-z]):/#/c/#g'
}

run_test() {
    local name="$1"
    shift

    local expected="$TMP_DIR/expected"
    local actual="$TMP_DIR/actual"

    grep "$@" > "$expected" 2> "$TMP_DIR/expected.err"
    local expected_status=$?

    "$GREP" "$@" > "$actual" 2> "$TMP_DIR/actual.err"
    local actual_status=$?

    normalize < "$expected" > "$TMP_DIR/expected.normalized"
    normalize < "$actual" > "$TMP_DIR/actual.normalized"

    if diff -u "$TMP_DIR/expected.normalized" \
              "$TMP_DIR/actual.normalized" > /dev/null &&
       [ "$expected_status" -eq "$actual_status" ]; then

        printf "PASS: %s\n" "$name"
        PASS=$((PASS + 1))
    else
        printf "FAIL: %s\n" "$name"

        if [ "$expected_status" -ne "$actual_status" ]; then
            printf "Expected exit code: %d\n" "$expected_status"
            printf "Actual exit code:   %d\n" "$actual_status"
        fi

        diff -u "$TMP_DIR/expected.normalized" \
               "$TMP_DIR/actual.normalized" || true

        FAIL=$((FAIL + 1))
    fi
}

cat > "$TMP_DIR/basic.txt" << 'EOF'
Hello world
hello C programming
HELLO test
another line
hello again
123 hello
hello hello
EOF

cat > "$TMP_DIR/second.txt" << 'EOF'
Hello second file
nothing here
hello again
another hello
EOF

cat > "$TMP_DIR/case.txt" << 'EOF'
hello
HELLO
Hello
world
123
EOF

cat > "$TMP_DIR/patterns.txt" << 'EOF'
hello
world
EOF

cat > "$TMP_DIR/empty_patterns.txt" << 'EOF'
EOF

printf "\n=== BASIC FLAGS ===\n"

run_test "basic search" \
    hello "$TMP_DIR/basic.txt"

run_test "-e" \
    -e hello "$TMP_DIR/basic.txt"

run_test "-i" \
    -i hello "$TMP_DIR/basic.txt"

run_test "-v" \
    -v hello "$TMP_DIR/basic.txt"

run_test "-c" \
    -c hello "$TMP_DIR/basic.txt"

run_test "-n" \
    -n hello "$TMP_DIR/basic.txt"

run_test "-l" \
    -l hello "$TMP_DIR/basic.txt"

printf "\n=== FLAG COMBINATIONS ===\n"

run_test "-i -n" \
    -i -n hello "$TMP_DIR/basic.txt"

run_test "-v -n" \
    -v -n hello "$TMP_DIR/basic.txt"

run_test "-c -n" \
    -c -n hello "$TMP_DIR/basic.txt"

run_test "-e -i" \
    -e hello -i "$TMP_DIR/basic.txt"

run_test "-e -v" \
    -e hello -v "$TMP_DIR/basic.txt"

run_test "-e -n" \
    -e hello -n "$TMP_DIR/basic.txt"

run_test "-e -c" \
    -e hello -c "$TMP_DIR/basic.txt"

run_test "-i -v" \
    -i -v hello "$TMP_DIR/basic.txt"

run_test "-i -c" \
    -i -c hello "$TMP_DIR/basic.txt"

run_test "-v -c" \
    -v -c hello "$TMP_DIR/basic.txt"

printf "\n=== COMPACT FLAG COMBINATIONS ===\n"

run_test "-iv" \
    -iv hello "$TMP_DIR/basic.txt"

run_test "-in" \
    -in hello "$TMP_DIR/basic.txt"

run_test "-vn" \
    -vn hello "$TMP_DIR/basic.txt"

run_test "-cn" \
    -cn hello "$TMP_DIR/basic.txt"

run_test "-ivn" \
    -ivn hello "$TMP_DIR/basic.txt"

printf "\n=== PATTERNS ===\n"

run_test "multiple patterns" \
    -e hello \
    -e world \
    "$TMP_DIR/basic.txt"

run_test "regex" \
    'hello.*' "$TMP_DIR/basic.txt"

run_test "regex with -i" \
    -i 'hello.*' "$TMP_DIR/basic.txt"

run_test "regex with -v" \
    -v '^hello' "$TMP_DIR/basic.txt"

printf "\n=== MULTIPLE FILES ===\n"

run_test "multiple files" \
    hello \
    "$TMP_DIR/basic.txt" \
    "$TMP_DIR/second.txt"

run_test "multiple files with -n" \
    -n hello \
    "$TMP_DIR/basic.txt" \
    "$TMP_DIR/second.txt"

run_test "multiple files with -c" \
    -c hello \
    "$TMP_DIR/basic.txt" \
    "$TMP_DIR/second.txt"

run_test "multiple files with -l" \
    -l hello \
    "$TMP_DIR/basic.txt" \
    "$TMP_DIR/second.txt"

printf "\n=== EXTRA FLAGS: -h ===\n"

run_test "-h" \
    -h hello \
    "$TMP_DIR/basic.txt" \
    "$TMP_DIR/second.txt"

run_test "-h -i" \
    -h -i hello \
    "$TMP_DIR/basic.txt" \
    "$TMP_DIR/second.txt"

run_test "-hi" \
    -hi hello \
    "$TMP_DIR/basic.txt" \
    "$TMP_DIR/second.txt"

run_test "-h -n" \
    -h -n hello \
    "$TMP_DIR/basic.txt" \
    "$TMP_DIR/second.txt"

printf "\n=== EXTRA FLAG: -s ===\n"

run_test "-s missing file" \
    -s hello "$TMP_DIR/missing.txt"

run_test "-s missing file with -i" \
    -si hello "$TMP_DIR/missing.txt"

printf "\n=== EXTRA FLAG: -f ===\n"

run_test "-f" \
    -f "$TMP_DIR/patterns.txt" "$TMP_DIR/basic.txt"

run_test "-f with -i" \
    -i -f "$TMP_DIR/patterns.txt" "$TMP_DIR/basic.txt"

run_test "-f with -n" \
    -n -f "$TMP_DIR/patterns.txt" "$TMP_DIR/basic.txt"

run_test "-f with multiple files" \
    -f "$TMP_DIR/patterns.txt" \
    "$TMP_DIR/basic.txt" \
    "$TMP_DIR/second.txt"

printf "\n=== EXTRA FLAG: -o ===\n"

run_test "-o" \
    -o hello "$TMP_DIR/basic.txt"

run_test "-o -i" \
    -o -i hello "$TMP_DIR/basic.txt"

run_test "-o -n" \
    -o -n hello "$TMP_DIR/basic.txt"

run_test "-o -i -n" \
    -o -i -n hello "$TMP_DIR/basic.txt"

run_test "-o regex" \
    -o 'hello.*' "$TMP_DIR/basic.txt"

printf "\n=== EXTRA FLAG COMBINATIONS ===\n"

run_test "-hi -n" \
    -hi -n hello \
    "$TMP_DIR/basic.txt" \
    "$TMP_DIR/second.txt"

run_test "-si" \
    -si hello "$TMP_DIR/missing.txt"

run_test "-i -f" \
    -i -f "$TMP_DIR/patterns.txt" "$TMP_DIR/basic.txt"

run_test "-o -f" \
    -o -f "$TMP_DIR/patterns.txt" "$TMP_DIR/basic.txt"

printf "\n=== CASE SENSITIVITY ===\n"

run_test "case sensitivity" \
    hello "$TMP_DIR/case.txt"

run_test "case insensitive" \
    -i hello "$TMP_DIR/case.txt"

printf "\n=== NO MATCHES ===\n"

run_test "no matches" \
    nonexistent_pattern "$TMP_DIR/basic.txt"

printf "\nGREP TESTS: %d passed, %d failed\n" "$PASS" "$FAIL"

if [ "$FAIL" -ne 0 ]; then
    exit 1
fi

exit 0