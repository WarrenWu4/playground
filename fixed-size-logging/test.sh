#!/bin/bash

msg=(100, 1000, 10000, 100000, 1000000)
kb=(10, 100, 1000, 10000, 100000)

command="make run"

for ((i=0; i<5; i++)); do
  arg1="${array1[$i]}"
  arg2="${array2[$i]}"
  output_file="tests/out_${i}_${arg1}_${arg2}.txt"
  eval "${command} MESSAGES=${arg1} KB=${arg2}" > "${output_file}"
  echo "Iteration $((i)) complete"
done

echo "Script complete"
