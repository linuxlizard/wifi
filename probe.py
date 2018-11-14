#!/usr/bin/env python3

# davep 20180225 ; learning scapy 
#	send a probe request, read a probe response

from scapy.all import *

# from wireless.h
SIOCGIWNAME=0x8b01

def decode_ie(id_, len_, info):
	# decode a single scapy Dot11Elt
	if id_ == 1:
		# basic rates
		# high bit indicates mandatory
		rates = [ ((n&0x80)>>7,n&0x7f) for n in info]
		print(rates)
	elif id_ == 0x30:
		# RSN
		pass
	elif id_ == 0xdd:
		# vendor specific
		print("oui info=%s" % info)
		oui = info[0:3]
		print("oui={:02x}{:02x}{:02x}".format(oui[0], oui[1], oui[2]))

def main():
	interfaces = get_if_list()
	print([(n,get_if_addr(n)) for n in interfaces])

	print([(n,get_if_raw_hwaddr(n)) for n in interfaces])
	print([(n,get_if_raw_addr(n)) for n in interfaces])

#	mac = [get_if_hwaddr(n) for n in interfaces]
#	print(list(zip(interfaces,mac)))

	wireless = []
	for intf in interfaces:
		try:
			buf = get_if(intf, SIOCGIWNAME)
			wireless.append(intf)
		except OSError:
			pass

	print("wireless interfaces=%s" % wireless)
	iface = wireless[0]
	fam, hw = get_if_raw_hwaddr(iface)
	print("hw=%s" % hw)

	# learning how to send/recv Dot11 packets via scapy/modules/krack
	bcast = "ff:ff:ff:ff:ff:ff"
#	dest = "ff:ff:ff:ff:ff:ff"
#	dest = "ac:a3:1e:f8:f1:10"
#	dest = "b0:6e:bf:db:bd:e8"
#	dest = "00:30:44:27:63:42"
	dest = "00:c0:ca:95:8b:27"
	dest = "00:30:44:1C:29:BF"   # wonderlab
	rep = RadioTap(len=None)\
		/ Dot11(
#		addr1=bcast,
		addr1=dest,
		addr2=hw,
		addr3=dest,
#		addr3=bcast,
#		FCfield = "from-DS",
		SC=9999
#		type=0,
#		subtype=4
		)\
		/ Dot11ProbeReq()\
		/ Dot11Elt(ID="SSID",len=0)\
		/ Dot11Elt(ID="Rates", info="\x0c\x12\x18\x24\x30\x48\x60\x6c")
	rep.show()
	rep.show2()
	print("summary=%s" % rep.summary())
	print("last=%s" % rep.lastlayer())
	hexdump(rep)

	ans, unans = srp(rep, iface=iface, timeout=5, filter="wlan addr1 a4:c4:94:a2:91:17")
#	ans, unans = srp(rep, multi=True, iface=iface, timeout=5, filter="wlan addr1 52:17:54:be:3f:00")
#	p = sr1(rep, iface=iface, timeout=5, filter="type mgt subtype probe-resp")
	print(ans)
	if ans is not None:
		ans.summary()
		print("stats=", ans.stats)
		print("res=", ans.res)
		print("listname=", ans.listname)
		print(type(ans), len(ans))
		for p in ans:
			print("type=%s len=%d" % (type(p), len(p)))
			print("p=",p)
			# [0] is the request
			# [1] is the response triggered by that request
			# so we need to take apart [1]
#			print("type(p[0]=%r)" % type(p[0]))
#			print("type(p[1]=%r)" % type(p[1]))
#			print("p[0]=%r" % p[0])
#			print("p[1]=%r" % p[1])
			resp = p[1]
			resp.show()
			print(resp.command())
			print(resp.sprintf("{Dot11ProbeResp:%Dot11.addr3%\t%Dot11ProbeResp.info%\t%Dot11ProbeResp.cap%}"))
			d = resp[Dot11]
#			print(type(d),dir(d))
			print("%s %d %d" %(d.name, d.type, d.subtype))
			cap = resp[Dot11ProbeResp].cap
			print(cap)
			elt = resp[Dot11Elt]
			print("elt=%r"%elt)
			print(dir(elt))
#			elt.show()
			print("elt len=%d" % (len(elt),))
			print("elt fields=%s" % (elt.fields,))
			print("elt info=%s" % (elt.info,))

			first = elt.firstlayer()
			last = elt.lastlayer()
			print("first=%r %d last=%r %d" % (first, id(first), last, id(last)))
			i=0
			while True:
				try:
					one_elt = elt[i]
				except IndexError:
					print("layer end at i=%d" % i)
					break
				i += 1
				print("i=%d %#x %d %s" %(i, one_elt.ID, one_elt.len, one_elt.info))
				decode_ie(one_elt.ID, one_elt.len, one_elt.info)
#				first = one_elt.firstlayer()
#				elt = elt.lastlayer()

#			print("payload=%r" %resp.payload)
			print("fields=",resp.fields)
#			for i in range(100):
#				print("i=%d %r" %(i,resp[i]))

if __name__ == '__main__':
	main()
