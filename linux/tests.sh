#!/bin/bash

make

tests=( SIGTERM SIGINT SIGUSR1 SIGHUP SIGQUIT SIGALRM )

for test in "${tests[@]}"
do
    printf "\n----- Running %s test -----\n\n" "$test"
    bash "tests/$test.sh"
    pkill echoserver -9
    rm test.log test.out test.daemon fifo 2> /dev/null
done

printf "\n"

make clean


