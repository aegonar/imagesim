#!/bin/bash
FILES=./n03970156/images/*
for f in $FILES
do
#  echo "Processing $f file..."
  # take action on each file. $f store current file name
  filename=$( echo $f | awk -F/ '{print $NF}' )
#  echo $filename
  ./Debug2/imagesim $filename
done
