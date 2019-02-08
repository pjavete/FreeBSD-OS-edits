#!/bin/bash

for i in {0..19}; do
    echo "Running ./benchmark $i"
    ./benchmark $i
done
