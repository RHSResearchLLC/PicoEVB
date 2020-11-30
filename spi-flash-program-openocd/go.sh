#!/bin/sh

date

# Unload problematic drivers
sudo rmmod ftdi_sio > /dev/null
sudo rmmod usbserial > /dev/null

# Program flash
sudo ./openocd -f flash.cfg

#Done
date






