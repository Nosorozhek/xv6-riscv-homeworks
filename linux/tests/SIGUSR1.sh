#!/bin/bash


if [ -f fifo ]; then
    mkfifo fifo
fi

pkill echoserver -9

./echoserver -n 1 -l test.log -o test.daemon > test.out 2>&1 &

yes "Hello World!" | dd bs=1 count=13 of=fifo status=none & # With \n at the end
sleep 0.1
pkill echoserver -USR1
sleep 0.1
yes "Hello World!" | dd bs=1 count=12 of=fifo status=none & # Without \n at the end
sleep 0.1
pkill echoserver -TERM
sleep 0.1

numprocesses=$(pidof echoserver | wc -l)
if [[ ! $numprocesses -eq 0 ]]; then
    printf "SIGUSR1 test failed.\n"
    pkill echoserver
    exit 1
fi

if [ ! -f test.log ]; then
    printf "SIGUSR1 test failed. Log output file was not found.\n"
    exit 1
fi

expected="tests/SIGUSR1.log"
actual="./test.log"
if !(cmp "$expected" "$actual"); then
    printf "SIGUSR1 test failed.\n"
    printf "Expected log:\n"
    cat "$expected"
    printf "Actual:\n"
    cat "$actual"
    exit 1
fi

expected="tests/SIGUSR1.out"
actual="./test.out"
if !(cmp "$expected" "$actual"); then
    printf "SIGUSR1 test failed.\n"
    printf "Expected output:\n"
    cat "$expected"
    printf "Actual:\n"
    cat "$actual"
    exit 1
fi


printf "SIGUSR1 test passed.\n"
exit 0
