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

# List of allowed model numbers
knownModels=(
	"BCM2807"
	"BCM2835"    # This appears to be returned for all higher BCM versions
	#"BCM2836"   # If that's not the case, enable these as well
	#"BCM2837"
	#"BCM2837B0"
)


model=$(cat /proc/cpuinfo | grep Hardware)
ret=$?
if [ $ret -eq 0 ]
then
	for knownModel  in "${knownModels[@]}"
	do
		if echo $model | grep $knownModel
		then
			echo This is a Pi platform
			exit 0
		fi
	done
fi


echo This is not a Pi platform
exit 1
