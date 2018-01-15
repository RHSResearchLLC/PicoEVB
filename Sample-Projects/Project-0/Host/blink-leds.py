#!/usr/bin/env python3
import os
import struct
import time

##############################################    
def main():


    # Helper constants
    all_1s = struct.pack(">I", 0xFFFFFFFF)
    all_0s = struct.pack(">I", 0x0)

    fd = os.open("/dev/xdma0_user", os.O_RDWR)

    # Make all outputs
    os.pwrite(fd, all_0s, 0x100C)

    for inx in range(30):
        time.sleep(0.5)
        if inx & 1:
            os.pwrite(fd, all_1s, 0x1008)
        else:
            os.pwrite(fd, all_0s, 0x1008)

    os.close(fd)


##############################################    

if __name__ == '__main__':
    main()
