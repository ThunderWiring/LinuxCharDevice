#!/usr/bin/bash

MODULE='my_module'
DEVICE='my_device0'
MY_DEVICE='my_device'
MINOR=0
echo "---------------installing your char device---------------"

#remove the device if exist:
rm -f /dev/$DEVICE
rmmod $MODULE 

#install the device:
echo "making your code:"
echo "executing Makefile..."
mae clean
make

insmod ./my_module.o

MAJOR=`cat /procs/devices | grep $MY_DEVICE | cut -d' ' -f1`
echo "my Major is "  $MAJOR
mknod /dev/$DEVICE c $MAJOR $MINOR

echo "---------------installation done!---------------"