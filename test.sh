#!/bin/bash

# Concurrent requests

rm *.txt

for (( i = 0; i < 10; i++ )); do
	curl 127.0.0.1:8888/ > Response$i.txt &
done

wait

all_files_match=true

for file1 in ./Response*.txt; do
	for file2 in ./Response*.txt; do
		if !(cmp -s "$file1" "$file2"); then
			all_files_match=false
		fi
	done
done

if $all_files_match; then
	echo Concurrent requests passed
else
	echo Concurrent requests failed
fi

rm *.txt