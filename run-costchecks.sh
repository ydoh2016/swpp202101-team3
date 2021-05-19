#!/bin/bash

if [ "$#" -ne 0 ]; then
  echo "./run-costchecks.sh"
  exit 1
fi

# set -e

echo "--- Start CostCheck.. ---"
echo ""

failed_inputs=()

for p in `ls -1 ./benchmarks`; do
    echo "== Testing Benchmark ${p} =="

    SRCFILE=`find ./benchmarks/${p}/src -name "*.ll"`

    timeout 60 bin/sf-compiler $SRCFILE .tmp.s

    CODE=$?

    if [[ $CODE -eq -124 ]]; then
        echo "TIMEOUT" 1>&2
        exit 1
    elif [[ $CODE -ne 0 ]]; then
        echo "COMPILER FAILURE" 1>&2
        exit 1
    else
        echo "COMPILE_TIME: PASSED"
    fi

    echo ""

    COUNT=1

    for i in `find ./benchmarks/${p}/test -name "input*.txt" | sort` ; do
        echo "--input$COUNT.txt--"

        DIFF=`diff -w ./benchmarks/${p}/test/output${COUNT}.txt <(bin/sf-interpreter .tmp.s < $i)`

        if [ -n "${DIFF}" ]; then
            echo "$p OUTPUT${COUNT}_TEST: FAILED"
            failed_inputs+=("$i")
        fi

        echo "$p OUTPUT${COUNT}_TEST: PASSED"

        COUNT=$((COUNT+1))
    done

done

if [[ "${#failed_inputs[@]}" -ne 0 ]]; then
  echo "== Failed Input List =="
  for i in "${failed_inputs[@]}"; do
    echo $i
  done
  echo "OUTPUT TEST FAILURE" 1>&2
  exit 1
else
  echo "== All Tests are PASSED! =="
fi