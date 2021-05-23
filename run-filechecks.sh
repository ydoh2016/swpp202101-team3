#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "run-filecheck.sh <FileCheck path>"
  exit 1
fi

echo "--- Start FileCheck.. ---"
echo ""

failed_opts=()

for p in `ls -1 ./filechecks`; do
  echo "== Testing Optimizer ${p} =="
  PASSED=0
  TOTAL=0
  for i in `find ./filechecks/${p} -name "*.ll"` ; do
    echo $i
    timeout 60 bin/sf-compiler $i .tmp.s ${p}

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

    $1 $i < .tmp.s
    if [[ "$?" -eq 0 ]]; then
      echo 'FILE_CHECK: PASSED'
      PASSED=$((PASSED+1))
    else
      failed_opts+=("$i")
    fi
    TOTAL=$((TOTAL+1))
  done
  echo ""
  echo "${p} passed: $PASSED / $TOTAL"
  echo ""
done

if [[ "${#failed_opts[@]}" -ne 0 ]]; then
  echo "== Failed Test List =="
  for i in "${failed_opts[@]}"; do
    echo $i
  done
  echo "FILECHECK FAILURE" 1>&2
  exit 1
else
  echo "== All FileCheck Tests are PASSED! =="
fi