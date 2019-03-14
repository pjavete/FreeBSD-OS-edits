#!/bin/bash
FILE="file"
# begin testing FreeBSD filesystem
echo "Testing FreeBSD file system"
COUNT=100
start=$(date +%s)
while [ $COUNT -gt 0 ]; do
	FILENAME="$FILE$COUNT"
	touch $FILENAME
	let COUNT=COUNT-1
done 
end=$(date +%s)
runtime=$end-$start
echo "Took $runtime seconds to create 100 files"
echo "Removing files"
COUNT=100
while [ $COUNT -gt 0 ]; do
	FILENAME="$FILE$COUNT"
	rm $FILENAME
	let COUNT=COUNT-1
done