#!/bin/bash
#
# Shell script to determine if this is running
# on an RPI.
#
# Returns 0 if this is an Rpi, 1 otherwise.
#
###############################################

file=/sys/firmware/devicetree/base/model

#
# This works for later version of Pi (distro's)
#
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


#
# This should work for all Pi's.
#
# Detect if this is a VideoCore. This should be sufficient for detecting Pi,
# since it's the only thing to date(!) using this particular chip version.
#
# The hardware from Pi 2 onward is actually BCM2836, but the call still returns BCM2835
#
model=$(cat /proc/cpuinfo | grep Hardware | grep BCM2835 )
ret=$?
if [ $ret -eq 0 ]
then
	echo This is a Pi platform
	exit 0
fi


echo This is not a Pi platform
exit 1
