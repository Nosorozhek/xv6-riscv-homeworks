#!/bin/bash

pkill echoserver -9
./echoserver -n 1 -l test.log -o test.daemon > test.out 2>&1 &

sleep 1.1

pkill echoserver -INT

sleep 0.1

numprocesses=$(pidof echoserver | wc -l)

if [[ ! $numprocesses -eq 0 ]]; then
    printf "SIGINT test failed.\n"
    exit 1
fi

expected="tests/SIGINT.out"
actual="test.out"
if ! (cmp "$expected" "$actual"); then
    printf "SIGINT test failed.\n"
    printf "Expected output:\n"
    cat "$expected"
    printf "Actual:\n"
    cat "$actual"
    rm test.out
    exit 1
fi

rm test.out

printf "SIGINT test passed.\n"
exit 0
