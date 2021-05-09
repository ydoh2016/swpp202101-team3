#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "run-filecheck.sh <FileCheck path>"
  exit 1
fi

echo "--- Start FileCheck.. ---"
set -e

failed_opts=()

for p in `ls -1 ./filechecks`; do
  echo "== Testing Optimizer ${p} =="
  for i in `find ./filechecks/${p} -name "*.ll"` ; do
    echo $i
    bin/sf-compiler $i .tmp.s
    $1 $i < .tmp.s
    if [ "$?" -eq 0 ]; then
      echo 'PASSED'
    else
      failed_opts+=("$i")
    fi
  done
done



if [[ "${#failed_opts[@]}" -ne 0 ]]; then
  echo "== Failed Test List =="
  for i in "${failed_opts[@]}"; do
    echo $i
  done
else
  echo "== All Tests are PASSED! =="
fi