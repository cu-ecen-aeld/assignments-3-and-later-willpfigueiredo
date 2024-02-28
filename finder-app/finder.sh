#!/bin/bash

num_par=$#

if [ ! $num_par -eq 2 ]; then
    echo "use ./finder.sh PATH STRING"
    exit 1;
fi

filesdir=$1
searchstr=$2

if [ ! -d $filesdir ]; then
    echo "PATH is not valid"
fi

files_count=$(ls -1 "$filesdir" | wc -l)
matching_count=$(grep -r "$searchstr" $filesdir | wc -l)

echo "The number of files are $files_count and the number of matching lines are $matching_count"
