#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "run-filecheck.sh <FileCheck path>"
  exit 1
fi

echo "--- Start FileCheck.. ---"
set -e

for p in adce dae licm simplifyCFG tailCallElim; do
for i in `find ./filechecks/${p} -name "*.ll"` ; do
  echo $i
  bin/sf-compiler $i .tmp.s ${p}
  $1 $i < .tmp.s
done
