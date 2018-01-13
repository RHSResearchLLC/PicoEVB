#!/bin/sh

set -e

# Make sure only root can run our script
if ! [ $(id -u) = 0 ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

echo 'building driver and tests'
cd Xilinx_Answer_65444_Linux_Files
(cd driver && make)
(cd tests && make)

echo 'Installing kernel driver'
cp etc/udev/rules.d/* /etc/udev/rules.d/
cp driver/xdma.ko /lib/modules/`uname -r`/kernel/drivers/pci
grep -q -F 'xdma' /etc/modules || echo 'xdma' >> /etc/modules
depmod
modprobe xdma

echo 'done'


