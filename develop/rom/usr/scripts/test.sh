#!/bin/bash
echo "hello,world!"
echo "bash yes!"
ps
pwd
for loop in 1 2 3 4 5
do
    echo "The value is: $loop"
done
int=1
while(( $int<=5 ))
do
    echo $int
    let "int++"
done