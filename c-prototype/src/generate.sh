#!/bin/bash
set -e

mkdir -p ../testing/raw_algo_measurements/ ../matrices/

for NUM in 2 3 10 20 30 40 50 100 200 500 1000
do
	./MultiPassWPA3_run $NUM 1 1
done
