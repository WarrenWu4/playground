#!/bin/bash

kb=(100, 1000, 10000)
msg=(100, 10000, 100000)

command="make run"

for ((i=0; i<3; i++)); do
  arg1="${kb[$i]}"
  echo "Starting kb=${arg1}"
  echo "--------------------------------"
  for ((j=0; j<3; j++)); do
    arg2="${msg[$j]}"
    echo "Starting msg=${arg2}"
    output_file="tests/out_${arg1}_${arg2}.txt"
    eval "${command} MESSAGES=${arg1} KB=${arg2}" > "${output_file}"
  done
  echo ""
done

echo "Script complete"
