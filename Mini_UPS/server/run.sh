#!/bin/bash

make clean
echo "Cleaned project."

make
echo "Compiled project."

./main &
echo "main exe"
python3 play.py
echo "Executed."

while true
do
sleep 1
done