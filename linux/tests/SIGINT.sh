#!/bin/bash


if [[ -f fifo ]]; then
    rm fifo
fi

mkfifo fifo

pkill echoserver -9

yes "Hello World!" | tr -d '\n' > fifo &

./echoserver -n 1 -l test.log -o test.daemon > test.out 2>&1 &

sleep 0.1

server_pid=$(pidof echoserver)

pkill "$server_pid" -INT

filesize=$(stat -c%s test.log)
sleep 0.2

filesize1=$(stat -c%s test.log)
filesize_ratio=$(echo "scale=2; $filesize1 / $filesize" | bc)

number_of_processes=$(pidof echoserver | wc -l)
if [[ ! $number_of_processes -eq 1 ]]; then
    printf "SIGINT test failed. Server has already exited.\n"
    rm test.log
    exit 1
fi

pkill yes

pkill "$server_pid" -9

rm test.log

if (( $(echo "$filesize_ratio < 2" |bc -l) )); then
    printf "SIGINT test failed. Log output file is too small.\n"
    exit 1
fi

printf "SIGINT test passed.\n"
pkill "$server_pid"
exit 0
