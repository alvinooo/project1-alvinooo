#!/bin/bash

# echo 'Please enter the server IP address'
# read addr
# echo $addr
# echo 'Please enter the server port'
# read port
# echo $port
# echo 'Please enter a concurrency level'
# read concurrent_requests
# echo $concurrent_requests

addr='128.54.70.250'
port='8888'
concurrent_requests=5

# Latency tests
for (( i = 0; i < $concurrent_requests; i++ )); do
	httping -c1 -g $addr:$port > Latency$i.txt &
done

# Throughput tests
for (( i = 0; i < $concurrent_requests; i++ )); do
	httping -c1 -Gg $addr:$port > Throughput$i.txt &
done

wait

python ext3.py $concurrent_requests

rm *.txt