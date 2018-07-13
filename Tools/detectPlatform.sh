#!/bin/bash
#
# Shell script to determine if this is running
# on an RPI.
#
# Returns 0 if this is an Rpi, 1 otherwise.
#
###############################################

file=/sys/firmware/devicetree/base/model

# This works for later version of Pi (distro's)
if false; then

if [ -f $file ]
then
	model=$(tr -d '\0' < $file)  # as `cat`, but avoid warning 'ignored null byte in input'
	ret=$?

	if [ $ret -eq 0 ] 
	then
		platform=$(echo $model | grep -q "^Raspberry Pi")
		ret=$?
		if [ $ret -eq 0 ] 
		then
			echo Platform: $model
			exit 0
		fi
	fi
fi

fi

# This should work for all Pi's
# The hardware from Pi 2 onward is actually BCM2836, but the cat-call still returns BCM2835
model=$(cat /proc/cpuinfo | grep Hardware | grep BCM2835 )
ret=$?
if [ $ret -eq 0 ]
then
	echo model checks out
	exit 0
fi


echo This is not an RPi platform
exit 1
