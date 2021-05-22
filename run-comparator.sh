#!/bin/bash

if [ "$#" -ne 0 ]; then
  echo "./run-comparator.sh"
  exit 1
fi

keys=(`jq 'keys[]' result.json`)

printf "%22s|" ""
printf "%22s|" "" >> table.txt

total=()

for key in "${keys[@]}"; do
    printf "%22s|" "${key}" >> table.txt
    printf "%22s|" "${key}"
    total+=(0)
done
truncate -s-1 table.txt
printf "\n" >> table.txt
printf "\n"

for benchmark in `jq ".no | keys[]" result.json`; do
    CNT=0
    for inp in `jq ".no.${benchmark} | keys[]" result.json`; do
        printf "%22s|" "${benchmark}-${CNT}" >> table.txt
        printf "%22s|" "${benchmark}-${CNT}"
        count=0
        for key in "${keys[@]}"; do
            cost=`jq ".${key}.${benchmark}.${inp}" result.json`
            oriCost=`jq ".no.${benchmark}.${inp}" result.json`
            diff=`echo $oriCost - $cost | bc -l`
            # echo "${diff}"
            printf "%22s|" "${cost}" >> table.txt
            printf "%22s|" "${cost}"
            total[$count]=`echo ${total[$count]} + $diff | bc -l`
            count=$((count+1))
        done
        truncate -s-1 table.txt
        printf "\n" >> table.txt
        printf "\n"
        CNT=$((CNT+1))
    done
done

