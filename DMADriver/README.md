
Official Windows and Linux driver support can be found here:

https://www.xilinx.com/support/answers/65444.html

The code hosted here is a copy of the files fetched from the link hosted above- with one change:

Later version of the Linux kernel don't have pci_enable_msix(), so the call was replaced with pci_enable_msix_exact()

Also, added a helper script to automatically build/compile/install the driver on Linux, so that the driver is automatically reloaded on powerup. Tested on Ubuntu 17.10




