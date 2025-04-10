#!/bin/bash

make

# SIGTERM also tests adding '\n' to the end of messages
# SIGHUP also tests ignoring SIGQUIT
tests=( SIGTERM SIGINT SIGUSR1 SIGHUP )

for test in "${tests[@]}"
do
    bash "tests/$test.sh"
    pkill echoserver -9
    rm test.log test.out test.daemon fifo 2> /dev/null
done

make clean


