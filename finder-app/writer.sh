#!/bin/bash

num_par=$#

if [ ! $num_par -eq 2 ]; then
    echo "use ./writer.sh FILE_PATH STRING"
    exit 1;
fi

filepath=$1
writestr=$2

IFS='/' read -a array <<< "$filepath"
filepath=""
for word in "${array[@]}"; do
    if [ "$word" = "." -o "$word" = "" -o "$word" = "~" ]; then
        filepath="$word"
    else
        filepath="$filepath/$word"
        if [ "$word" = "${array[-1]}" ]; then
           echo "$writestr" > "$filepath"
        else
          if [ ! -d "$filepath" ]; then 
              mkdir "$filepath"
          fi
        fi
    fi
done
    


