#!/bin/bash
set +e

cmake CMakeLists.txt
mkdir -p testing/raw_algo_measurements/

cd src
make
cd ..

cd bench
make
cd ..

cd tst
make
cd ..

# Generate values
cd src
for NUM in {2..200}; do
	echo "Generating vals $NUM/202"
	./MultiPassWPA3_run $NUM 1 1
done
echo "Generating vals 201/202"
./MultiPassWPA3_run  500 1 1
echo "Generating vals 202/202"
./MultiPassWPA3_run 1000 1 1

# Generate matrices

