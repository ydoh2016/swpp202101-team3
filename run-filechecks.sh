#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "run-filecheck.sh <FileCheck path>"
  exit 1
fi

echo "--- Start FileCheck.. ---"
#set -e

for i in `find ./filechecks -name "*.ll"` ; do
  ./sf-compiler $i .tmp.s
  $1 $i < .tmp.s
done
