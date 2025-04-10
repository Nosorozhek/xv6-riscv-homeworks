#!/bin/bash

pkill echoserver -9

./echoserver -n 1 -l test.log -o test.daemon > test.out 2>&1 &

sleep 0.1

pkill echoserver -HUP

sleep 0.1

yes "Hello World!" | tr -d '\n' | dd bs=1 count=10 of=fifo status=none &
sleep 0.1
pkill echoserver -USR1
pkill echoserver -QUIT # Must be ignored
pkill echoserver -ALRM # Invoke the alarm manually
sleep 0.1
pkill echoserver -INT
sleep 0.1

number_of_processes=$(pidof echoserver | wc -l)
if [[ ! $number_of_processes -eq 0 ]]; then
    printf "SIGHUP test failed. Server didn't exit after SIGINT.\n"
    exit 1
fi

expected="tests/SIGHUP.daemon"
actual="test.daemon"
if ! (cmp "$expected" "$actual"); then
    printf "SIGHUP test failed.\n"
    printf "Expected output from daemonized process:\n"
    cat "$expected"
    printf "Actual:\n"
    cat "$actual"
    exit 1
fi

expected="tests/SIGHUP.log"
actual="test.log"
if ! (cmp "$expected" "$actual"); then
    printf "SIGHUP test failed.\n"
    printf "Expected log:\n"
    cat "$expected"
    printf "Actual:\n"
    cat "$actual"
    rm "$actual"
    exit 1
fi

printf "SIGHUP test passed.\n"
exit 0
