This directory contains the files necessary to program PicoEVB's flash using OpenOCD.

This directory contains a modified version of OpenOCD, which will set the flash's "quad enable" bit. 
However, this bit is set at the factory so this version of OpenOCD isn't necessary- you can use any recent stock version
and it should work on Windows or Linux.

If you already have OpenOCD installed:

Linux:
 - Make sure PicoEVB is installed and powered up
 - Run go.sh


Windows
 - Make sure PicoEVB is installed and powered up
 - Use the SysProgs USB Driver Tool on Windows to load the WinUSB Driver for the FT230X
 - Run openocd.exe -f flash.cfg
