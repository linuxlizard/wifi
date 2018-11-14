#!/usr/bin/env python3

# davep 201800303; learning scapy 
#	broadcast a probe request, read probe response(s)

from scapy.all import *

def main(iface):
	fam, hw = get_if_raw_hwaddr(iface)
	print("hw=%s" % hw)

	bcast = "ff:ff:ff:ff:ff:ff"
	rep = RadioTap(len=None)\
		/ Dot11(
		addr1=bcast,
		addr2=hw,
		addr3=bcast,
#		SC=9999
		)\
		/ Dot11ProbeReq()\
		/ Dot11Elt(ID="SSID",len=0)\
		/ Dot11Elt(ID="Rates", info="\x0c\x12\x18\x24\x30\x48\x60\x6c")

#	ans, unans = srp(rep, multi=True, iface=iface, timeout=5)
	ans, unans = srp(rep, multi=True, iface=iface, timeout=5, filter="wlan addr1 a4:c4:94:a2:91:17")

	if ans is not None:
		ans.summary()

if __name__ == '__main__':
	iface = sys.argv[1]
	main(iface)
