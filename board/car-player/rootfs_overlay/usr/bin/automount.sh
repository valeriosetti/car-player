#!/bin/bash

LOG_FILE=/dev/kmsg

echo "script started as:" $0 $1 $2 > $LOG_FILE

if [ "$#" -ne 2 ]; then
        echo "$0 - Error: wrong number of input parameters ($#)" > $LOG_FILE
        exit 1
fi
ACTION=$1
DEVICE=$2

case $ACTION in
        "add")
                echo "$0 - Mounting device: /dev/$DEVICE in /media/$DEVICE" > $LOG_FILE
                mkdir -p /media/$DEVICE
                mount /dev/$DEVICE /media/$DEVICE
                ;;

        "remove")
                echo "$0 - Removing mount point: /media/$DEVICE" > $LOG_FILE
                umount /media/$DEVICE
                ;;
esac
