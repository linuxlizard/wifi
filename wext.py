#!/usr/bin/env python3
# davep 20181107 ; wext-y goodness in Python. Experimental.

import socket
import fcntl
import struct
import math

IFNAMSIZE = 16

SIOCSIWNWID = 0x8b03
SIOCGIWFREQ = 0x8b05
SIOCGIWAP = 0x8b15
SIOCGIWPOWER = 0x8b2d
SIOCGIWMODE = 0x8b07
SIOCGIWRATE = 0x8b21

IEEE80211_IOCTL_GETMODE = 0x8bf1

def main():
        ifname = "wlp1s0"
#        ifname = "athc20"
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        buf = struct.pack("32s", ifname.encode("latin1"))
        print("%r" % buf)
        buf = bytearray(buf)
        outbuf = fcntl.ioctl(sock, SIOCGIWFREQ, buf)
        print("buf=%r" % buf)

        iwreq_data = buf[16:]
        print(iwreq_data)
        iw_freq = struct.unpack("<lhBB", iwreq_data[:8])
        print(iw_freq)
        # from iw_freq2float() in iwlib.c wireless tools
#       return ((double) in->m) * pow(10,in->e);
        m,e,i,flags = iw_freq
        print(float(m) * math.pow(10,e))

        buf = bytearray(struct.pack("32s", ifname.encode("latin1")))
        outbuf = fcntl.ioctl(sock, SIOCGIWMODE, buf)
        print("buf=%r" % buf)

        buf = bytearray(struct.pack("32s", ifname.encode("latin1")))
        outbuf = fcntl.ioctl(sock, SIOCGIWRATE, buf)
        print("buf=%r" % buf)

        iw_param = struct.unpack("<lBBH", buf[16:24])
        print(iw_param)

        # pass an iw_point?
#        dataspace = bytes(64)
#        buf = bytearray(struct.pack("16sPHH", ifname.encode("latin1"), dataspace, 64, 0 ) )
#        outbuf = fcntl.ioctl(sock, IEEE80211_IOCTL_GETMODE, buf+dataspace)
#        print("buf=%r" % buf)

if __name__ == '__main__':
        main()
