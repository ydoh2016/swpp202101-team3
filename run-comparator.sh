#!/bin/bash

if [ "$#" -ne 0 ]; then
  echo "./run-comparator.sh"
  exit 1
fi

keys=(`jq 'keys[]' result.json`)

rm -rf table.txt

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

for benchmark in `jq ".sprint1 | keys[]" result.json`; do
    CNT=1
    for inp in `jq ".sprint1.${benchmark} | keys[]" result.json`; do
        printf "%22s|" "${benchmark}-${CNT}" >> table.txt
        printf "%22s|" "${benchmark}-${CNT}"
        count=0
        for key in "${keys[@]}"; do
            cost=`jq ".${key}.${benchmark}.${inp}" result.json`
            oriCost=`jq ".sprint1.${benchmark}.${inp}" result.json`
            diff=`echo $oriCost - $cost | bc -l`
            printf "%22s|" "${diff}" >> table.txt
            printf "%22s|" "${diff}"
            total[$count]=`echo ${total[$count]} + $diff | bc -l`
            count=$((count+1))
        done
        truncate -s-1 table.txt
        printf "\n" >> table.txt
        printf "\n"
        CNT=$((CNT+1))
    done
done

printf "%22s|" "total" >> table.txt
printf "%22s|" "total"

for t in "${total[@]}"; do
    printf "%22s|" "${t}" >> table.txt
    printf "%22s|" "${t}"
done

if (( $(echo "${total[0]} < 0" |bc -l) )); then
    echo ""
    echo ""
    echo "COST LARGER THAN BEFORE!" 1>&2
    exit 1
else
    echo ""
    echo ""
    echo "====================== COST TEST PASSED ======================"
    exit 0
fi
