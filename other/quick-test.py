#
# Notes:
# Requires that the xvcd program does not need a sudo password

import argparse
import os
import socket
import subprocess
import struct
import sys
import time

# Test settings
DESIRED_IDCODE = 0x0362C093  # XC7A50T

# xvcd settings
TCP_IP = '127.0.0.1'
TCP_PORT = 2542
BUFFER_SIZE = 256
MESSAGE=bytes([0x73,0x68,0x69,0x66,0x74,0x3a,0x30,0x00,0x00,0x00,0xff,0x2f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00])

# Helper constants
all_1s = struct.pack(">I", 0xFFFFFFFF)
all_0s = struct.pack(">I", 0x0)

##############################################    
def printf(format, *args):
    sys.stdout.write(format % args)

##############################################    
def loadDriver():
    subprocess.call('cd /home/dr/git/PicoEVB/DMADriver/Xilinx_Answer_65444_Linux_Files/tests && sudo ./load_driver.sh', shell=True)

##############################################    
def GetIDCode():
    id_code = 0xFFFFFFFF # default to bogus value

    # Launch Xilinx Virtual Cable server
    xvcd = subprocess.Popen('sudo /home/dr/git/xvcd/bin/xvcd -P 0x6015', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    time.sleep(0.5)

    # Now connect to server via TCP/IP, send some data, and wait for a response
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((TCP_IP, TCP_PORT))
    s.send(MESSAGE)
    data = s.recv(BUFFER_SIZE)
    s.close()

    if len(data) == 6:
        # Grab just the last 32 bits as an int
        id_code = struct.unpack("<L", data[2:6])[0]

    # And we are done
    xvcd.terminate();

    return id_code

def setLEDs(state):

    # Get the device
    fd = os.open("/dev/xdma0_user", os.O_RDWR)

    # Make port output
    os.pwrite(fd, all_0s, 0x0000100C)

    if state:
        os.pwrite(fd, all_0s, 0x00001008)
    else:
        os.pwrite(fd, all_1s, 0x00001008)

    os.close(fd)

##############################################    
def main():

    parser = argparse.ArgumentParser()
    parser.parse_args()

    loadDriver()

    # Get the IDCODE and print
    id_code = GetIDCode();
    printf ("IDCODE=%08x\n", id_code);

    # Mask off the upper 4 bits, which are the silicon revision
    id_code = id_code & 0x0FFFFFFF

    if id_code != DESIRED_IDCODE:
        printf("Bad ID code. Expected %x received %x\n", DESIRED_IDCODE, id_code)
        setLEDs(1); # All LEDs on
    else:
        printf("Tests pass\n")
        setLEDs(0); # All LEDs off


##############################################    

if __name__ == '__main__':
    main()
