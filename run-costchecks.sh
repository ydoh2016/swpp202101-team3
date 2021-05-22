#!/bin/bash

if [ "$#" -ne 0 ]; then
  echo "./run-costchecks.sh"
  exit 1
fi

# set -e

echo "--- Start CostCheck.. ---"
echo ""

failed_pass=()

echo '{}' | jq '.' > result.json

passes=("no" "all")
passes+=(`ls -1 ./filechecks`)

for pass in "${passes[@]}"; do
    failed_inputs=()
    echo "== Pass name ${pass} =="

    jq ".${pass} = {}" result.json > tmp.json
    cat tmp.json > result.json

    for p in `ls -1 ./benchmarks`; do

        echo "== Testing Benchmark ${p} =="

        SRCFILE=`find ./benchmarks/${p}/src -name "*.ll"`

        timeout 60 bin/sf-compiler $SRCFILE .tmp.s $pass

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

        jq ".${pass}.${p} = {}" result.json > tmp.json
        cat tmp.json > result.json

        COUNT=1

        for i in `find ./benchmarks/${p}/test -name "input*.txt" | sort` ; do
            echo "--input$COUNT.txt--"

            DIFF=`diff -w ./benchmarks/${p}/test/output${COUNT}.txt <(bin/sf-interpreter .tmp.s < $i)`

            COST=`grep "Cost" ./sf-interpreter.log | cut -f2 -d ":" | cut -f2 -d " "`

            if [ -n "${DIFF}" ]; then
                echo "$p OUTPUT${COUNT}_TEST: FAILED"
                failed_inputs+=("$i")
            fi

            echo "$p OUTPUT${COUNT}_TEST: PASSED"

            jq ".${pass}.${p}.input${COUNT} = ${COST}" result.json > tmp.json
            cat tmp.json > result.json

            COUNT=$((COUNT+1))
        done
    done
    if [[ "${#failed_inputs[@]}" -ne 0 ]]; then
        failed_pass+=("$pass") 
        echo "== Pass ${pass} Failed Input List =="
        for i in "${failed_inputs[@]}"; do
            echo $i
            failed_pass+=("$i")
        done
        echo "OUTPUT TEST FAILURE" 1>&2
    else
        echo "== All Tests are PASSED! =="
    fi
done

if [[ "${#failed_pass[@]}" -ne 0 ]]; then
    echo "== Failed Pass List =="
    for i in "${failed_pass[@]}"; do
        echo $i
        printf $i >> failed_pass.log
    done
    echo "OUTPUT TEST FAILURE" 1>&2
else
    echo "== All Tests are PASSED! =="
fi

rm -rf tmp.json
