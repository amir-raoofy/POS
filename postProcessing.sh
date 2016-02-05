#!/bin/bash

echo "File name?"
read -r filename

# Make sure file exists
if [ -r "$filename" ]; then
	echo $filename
	echo ""
	
	echo "Computation time"
	
	for i in 64 128 256 512 1024 2048 4096
	do
		echo "$i"
		sed -n '/('"$i"',/{;n;p}' $filename
	done

	echo "MPI time"

	for i in 64 128 256 512 1024 2048 4096
	do
		echo "$i"
		sed -n '/('"$i"',/{;n;n;p}' $filename
	done

	echo "Initialization time"

        for i in 64 128 256 512 1024 2048 4096
        do
                echo "$i"
                sed -n '/('"$i"',/{;n;n;n;p}' $filename
        done

	echo "output time"

        for i in 64 128 256 512 1024 2048 4096
        do
                echo "$i"
                sed -n '/('"$i"',/{;n;n;n;n;p}' $filename
        done



fi
