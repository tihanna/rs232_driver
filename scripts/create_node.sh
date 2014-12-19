#!/bin/sh
module=rs232_mod
major=$(awk -v m=$module '$2==m {print $1}' /proc/devices)
mknod /dev/rs232 c $major 0
