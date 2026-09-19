#!/usr/bin/env bash

set -u

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
CAT="$ROOT_DIR/src/cat/s21_cat"
TMP_DIR="$ROOT_DIR/tests/.tmp_cat"

PASS=0
FAIL=0

cleanup() {
    rm -rf "$TMP_DIR"
}

trap cleanup EXIT

mkdir -p "$TMP_DIR"

normalize() {
    tr -d '\r'
}

run_test() {
    local name="$1"
    shift

    local expected="$TMP_DIR/expected"
    local actual="$TMP_DIR/actual"

    cat "$@" > "$expected" 2> "$TMP_DIR/expected.err"
    "$CAT" "$@" > "$actual" 2> "$TMP_DIR/actual.err"

    if normalize < "$expected" | diff -u - <(normalize < "$actual") > /dev/null; then
        printf "PASS: %s\n" "$name"
        PASS=$((PASS + 1))
    else
        printf "FAIL: %s\n" "$name"
        normalize < "$expected" > "$TMP_DIR/expected.normalized"
        normalize < "$actual" > "$TMP_DIR/actual.normalized"
        diff -u "$TMP_DIR/expected.normalized" \
               "$TMP_DIR/actual.normalized" || true
        FAIL=$((FAIL + 1))
    fi
}

printf 'one\n\ntwo\n\tthree\n' > "$TMP_DIR/basic.txt"
printf '\n\none\n\n\n two\n' > "$TMP_DIR/blank.txt"
printf 'hello\tworld\nsecond line\n' > "$TMP_DIR/tabs.txt"
printf 'one\n\ntwo\n\nthree\n' > "$TMP_DIR/combo.txt"
printf '\001\002\007\010\014\016\017\177\n' > "$TMP_DIR/nonprinting.txt"

run_test "no flags" "$TMP_DIR/basic.txt"
run_test "-b" -b "$TMP_DIR/basic.txt"
run_test "-e" -e "$TMP_DIR/basic.txt"
run_test "-E" -E "$TMP_DIR/basic.txt"
run_test "-n" -n "$TMP_DIR/basic.txt"
run_test "-s" -s "$TMP_DIR/blank.txt"
run_test "-t" -t "$TMP_DIR/tabs.txt"
run_test "-T" -T "$TMP_DIR/tabs.txt"
run_test "-v" -v "$TMP_DIR/nonprinting.txt"

run_test "-b -n" -b -n "$TMP_DIR/basic.txt"
run_test "-b -s" -b -s "$TMP_DIR/combo.txt"
run_test "-e -n" -e -n "$TMP_DIR/basic.txt"
run_test "-n -s" -n -s "$TMP_DIR/combo.txt"
run_test "-t -n" -t -n "$TMP_DIR/tabs.txt"
run_test "-T -n" -T -n "$TMP_DIR/tabs.txt"
run_test "-v -n" -v -n "$TMP_DIR/nonprinting.txt"

run_test "-benstTv" -benstTv "$TMP_DIR/combo.txt"

run_test "--number-nonblank" --number-nonblank "$TMP_DIR/basic.txt"
run_test "--number" --number "$TMP_DIR/basic.txt"
run_test "--squeeze-blank" --squeeze-blank "$TMP_DIR/blank.txt"

run_test "multiple files" "$TMP_DIR/basic.txt" "$TMP_DIR/blank.txt"

printf "\nCAT TESTS: %d passed, %d failed\n" "$PASS" "$FAIL"

if [ "$FAIL" -ne 0 ]; then
    exit 1
fi

exit 0