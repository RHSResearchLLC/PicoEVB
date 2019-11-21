#!/usr/bin/env python3
import os
import struct
import time
import sys

def printf(format, *args):
    sys.stdout.write(format % args)

def ConvTemp(raw_int):
    return (float(raw_int) * 503.975 / 65536.0) - 273.15;

def ConvVolt(raw_int):
    return (float(raw_int) * 3.0 / 65535.0)

##############################################    
def main():


    fd = os.open("/dev/xdma0_user", os.O_RDWR)

    # sysmon is at 0x3000; first register of interest (temperature) is at 0x200
    sysmon_base = 0x3200;

    vals = [];

    for reg in range(0x28):
        temp_string = os.pread(fd, 4, sysmon_base + (reg * 4));
        temp_int = int.from_bytes(temp_string, byteorder='little')
        vals.append(temp_int)

    # At this point, vals contains the integer values read from ADC, with the same index as the ADC register offset

    printf("           | CUR  | MIN  | MAX  |\n")
    printf("Temp C     | %.1f | %.1f | %.1f |\n", ConvTemp(vals[0])   , ConvTemp(vals[0x24]), ConvTemp(vals[0x20]))
    printf("VCCInt     | %.2f | %.2f | %.2f |\n", ConvVolt(vals[1])   , ConvVolt(vals[0x25]), ConvVolt(vals[0x21]))
    printf("VCCAux     | %.2f | %.2f | %.2f |\n", ConvVolt(vals[2])   , ConvVolt(vals[0x26]), ConvVolt(vals[0x22])) 

    os.close(fd)


##############################################    

if __name__ == '__main__':
    main()
