#!/bin/bash

pkill echoserver -9
./echoserver -n 1 -l test.log -o test.daemon > test.out 2>&1 &

sleep 2.1

pkill echoserver -HUP
sleep 2.1

pkill echoserver -TERM

numprocesses=$(pidof echoserver | wc -l)

if [[ ! $numprocesses -eq 0 ]]; then
    printf "SIGALRM test failed.\n"
    exit 1
fi

expected="tests/SIGALRM.out"
actual="test.out"
if ! (cmp "$expected" "$actual"); then
    printf "SIGALRM test failed.\n"
    printf "Expected output after 2 SIGALRMs:\n"
    cat "$expected"
    printf "Actual:\n"
    cat "$actual"
    rm test.out
    exit 1
fi

expected="tests/SIGALRM.out"
actual="test.out"
if ! (cmp "$expected" "$actual"); then
    printf "SIGALRM test failed.\n"
    printf "Expected output after 2 SIGALRMs:\n"
    cat "$expected"
    printf "Actual:\n"
    cat "$actual"
    rm test.out
    exit 1
fi

rm test.out

printf "SIGALRM test passed.\n"
exit 0
