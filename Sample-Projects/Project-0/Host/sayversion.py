#!/usr/bin/env python3
import os
import struct
import time

##############################################    
def main():


    fd = os.open("/dev/xdma0_user", os.O_RDONLY)

    temp_string = os.pread(fd, 4, 0x1000)

    print("Version " + str(temp_string))

    os.close(fd)


##############################################    

if __name__ == '__main__':
    main()
