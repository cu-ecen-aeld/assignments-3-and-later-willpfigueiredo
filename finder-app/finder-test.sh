#!/bin/bash
# Tester script for assignment 1 and assignment 2
# Author: Siddhant Jajoo

set -e
set -u

NUMFILES=10
WRITESTR=AELD_IS_FUN
WRITEDIR=/tmp/aeld-data
username=$(cat /etc/finder-app/conf/username.txt)

echo "Starting..."
retcmd=$(which writer &> /dev/null; echo $?)
echo "ret value $retcmd"

if [ $# -lt 3 ]
then
	echo "Using default value ${WRITESTR} for string to write"
	if [ $# -lt 1 ]
	then
		echo "Using default value ${NUMFILES} for number of files to write"
	else
		NUMFILES=$1
	fi	
else
	NUMFILES=$1
	WRITESTR=$2
	WRITEDIR=/tmp/aeld-data/$3
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

rm -rf "${WRITEDIR}"

# create $WRITEDIR if not assignment1
assignment=`cat /etc/finder-app/conf/assignment.txt`

if [ $assignment != 'assignment1' ]
then
	mkdir -p "$WRITEDIR"

	#The WRITEDIR is in quotes because if the directory path consists of spaces, then variable substitution will consider it as multiple argument.
	#The quotes signify that the entire string in WRITEDIR is a single string.
	#This issue can also be resolved by using double square brackets i.e [[ ]] instead of using quotes.
	if [ -d "$WRITEDIR" ]
	then
		echo "$WRITEDIR created"
	else
		exit 1
	fi
fi

for i in $( seq 1 $NUMFILES)
do
	
	if [ $retcmd -eq 0 ]
	then
		writer "$WRITEDIR/${username}$i.txt" "$WRITESTR"
	else
		./writer "$WRITEDIR/${username}$i.txt" "$WRITESTR"
	fi
done
if [ $retcmd -eq 0 ]
then
	OUTPUTSTRING=$(finder.sh "$WRITEDIR" "$WRITESTR")
else	
	OUTPUTSTRING=$(./finder.sh "$WRITEDIR" "$WRITESTR")
fi

# remove temporary directories
rm -rf /tmp/aeld-data

set +e
echo ${OUTPUTSTRING} | grep "${MATCHSTR}"
echo ${OUTPUTSTRING} > /tmp/assignment4-result.txt
if [ $? -eq 0 ]; then
	echo "success"
	exit 0
else
	echo "failed: expected  ${MATCHSTR} in ${OUTPUTSTRING} but instead found"
	exit 1
fi
