#!/usr/bin/env python3
# davep 20181117 ; dump enum numbers from nl80211.h */

import sys
import logging
import re
from enum import Enum
import io
from functools import singledispatch

logger = logging.getLogger("nl_nums")

class PState(Enum):
	INIT = 0
	CODE = 1
	COMMENT = 2

def readlines(infile):
	state = PState.CODE
	while True:
		line = infile.readline()
		if not line:
			break

		line = line.strip()
		if not line:
			# ignore blank lines 
			continue

		# very naive comment eater
		# nl80211.h is very cleanly formatted so this seems to work ok
		if line.startswith("//"):
			continue
		if state == PState.CODE:
			if line.startswith("/*"):
				if not line.endswith("*/"):
					state = PState.COMMENT
				continue
		elif state == PState.COMMENT:
			if line.endswith("*/"):
				state = PState.CODE
			continue
		else:
			assert 0, state

		yield line

class DecodedEnum:
	def __init__(self, name):
		self.name = name
		# dict of value -> name
		self.values = {}
		# dict of name -> value
		self.names = {}

	def add(self, name, value):
		logger.debug("add name=%s value=%d",name,value)
		self.names[name] = value
		self.values[value] = name

	def __getitem__(self, item):
		@singledispatch
		def get(item, self):
			return self.names[item]

		@get.register(int)
		def _(item, self):
			# get value -> name
			return self.values[item]

		@get.register(str)
		def _(item, self):
			# get name -> value
			return self.names[item]

		# functools.singledispatch works on the first arg so have to do a
		# little jiggery pokery
		# https://docs.python.org/3/library/functools.html
		# "Note that the dispatch happens on the type of the first argument,
		# create your function accordingly:"
		return get(item, self)

class EState(Enum):
	INIT = 0
	CODE = 1
	ENUM = 2

# C-style variable name
var = "[a-zA-Z_][a-zA-Z_0-9]*"

# using f strings so save myself some confusion 
open_brace = "\{"
close_brace = "\}"
whitespace = "\s+"
number = "-?[0-9]+"

# start of an emum declaration (XXX assumes open brace on same line as the
# 'enum' keywoard
enum_matcher = re.compile(f"enum{whitespace}({var}){whitespace}{open_brace}")

# enum member regex
symbol_matcher = re.compile(f"({var})({whitespace}={whitespace}{number})?")

def enum_counter(reader):
	enum = None
	counter = 0
	state = EState.CODE
	for line in reader:
		# super naive parser
		if state == EState.CODE:
			m = enum_matcher.search(line)
			if m:
				state = EState.ENUM
				name = m.group(1)
				logging.debug("found enum=%s", name)
				enum = DecodedEnum(name)

		elif state == EState.ENUM:
			if line.endswith("};"):
				logging.debug("end of enum=%s", enum.name)
				state = EState.CODE
				yield enum
				enum = None
				counter = 0
			else:
				m = symbol_matcher.search(line)
				if m:
					print("groups={}".format(m.groups()))
					name = m.group(1)
					logging.debug("found enum element=%s counter=%d", name, counter)
					enum.add(name, counter)
					counter += 1

		else:
			assert 0, state

def test_enum_regex():
	def search(s):
		m = enum_matcher.search(s)
		print(m)
		assert m, s
		print(m.group(1))
		assert m.group(1) == "nl80211_nan_srf_attributes", m.group(1)

	search("enum nl80211_nan_srf_attributes {")
	search("enum     nl80211_nan_srf_attributes    {")
	search("     enum\tnl80211_nan_srf_attributes    {")
	search("\tenum\tnl80211_nan_srf_attributes\t{")

def test_symbol_regex():
	def search(s, expected_value):
		m = symbol_matcher.search(s)
		print(m)
		assert m, s
		print(m.groups())
		print(m.group(1))
		assert m.group(1) == "NL80211_NAN_FUNC_TYPE", m.group(1)

	search("NL80211_NAN_FUNC_TYPE,", None)
	search("NL80211_NAN_FUNC_TYPE = 0,", 0)
	search("NL80211_NAN_FUNC_TYPE = 99,", 99)
	search("NL80211_NAN_FUNC_TYPE = -1,", -1)
	search("NL80211_NAN_FUNC_TYPE = NUM_NL80211_NAN_FUNC_ATTR - 1", None)

def test_parse():
	s = """
enum nl80211_nan_func_attributes {
	__NL80211_NAN_FUNC_INVALID,
	NL80211_NAN_FUNC_TYPE,
	NL80211_NAN_FUNC_SERVICE_ID,
	NL80211_NAN_FUNC_PUBLISH_TYPE,
	NL80211_NAN_FUNC_PUBLISH_BCAST,
	NL80211_NAN_FUNC_SUBSCRIBE_ACTIVE,
	NL80211_NAN_FUNC_FOLLOW_UP_ID,
	NL80211_NAN_FUNC_FOLLOW_UP_REQ_ID,
	NL80211_NAN_FUNC_FOLLOW_UP_DEST,
	NL80211_NAN_FUNC_CLOSE_RANGE,
	NL80211_NAN_FUNC_TTL,
	NL80211_NAN_FUNC_SERVICE_INFO,
	NL80211_NAN_FUNC_SRF,
	NL80211_NAN_FUNC_RX_MATCH_FILTER,
	NL80211_NAN_FUNC_TX_MATCH_FILTER,
	NL80211_NAN_FUNC_INSTANCE_ID,
	NL80211_NAN_FUNC_TERM_REASON,

	/* keep last */
	NUM_NL80211_NAN_FUNC_ATTR,
	NL80211_NAN_FUNC_ATTR_MAX = NUM_NL80211_NAN_FUNC_ATTR - 1

};


/**
 * enum nl80211_nan_srf_attributes - NAN Service Response filter attributes
 * @__NL80211_NAN_SRF_INVALID: invalid
 * @NL80211_NAN_SRF_INCLUDE: present if the include bit of the SRF set.
 *This is a flag.
 * @NL80211_NAN_SRF_BF: Bloom Filter. Present if and only if
 *&NL80211_NAN_SRF_MAC_ADDRS isn't present. This attribute is binary.
 * @NL80211_NAN_SRF_BF_IDX: index of the Bloom Filter. Mandatory if
 *&NL80211_NAN_SRF_BF is present. This is a u8.
 * @NL80211_NAN_SRF_MAC_ADDRS: list of MAC addresses for the SRF. Present if
 *and only if &NL80211_NAN_SRF_BF isn't present. This is a nested
 *attribute. Each nested attribute is a MAC address.
 * @NUM_NL80211_NAN_SRF_ATTR: internal
 * @NL80211_NAN_SRF_ATTR_MAX: highest NAN SRF attribute
 */

enum nl80211_nan_srf_attributes {
	__NL80211_NAN_SRF_INVALID,
	NL80211_NAN_SRF_INCLUDE,
	NL80211_NAN_SRF_BF,
	NL80211_NAN_SRF_BF_IDX,
	NL80211_NAN_SRF_MAC_ADDRS,

	/* keep last */
	NUM_NL80211_NAN_SRF_ATTR,
	NL80211_NAN_SRF_ATTR_MAX = NUM_NL80211_NAN_SRF_ATTR - 1,
};

"""
	infile = io.StringIO(s)
	enums = parse(infile)
	assert len(enums) == 2, len(enums)
	nanf = enums[0]
	assert nanf[0] == "__NL80211_NAN_FUNC_INVALID", nanf[0]
	assert nanf["__NL80211_NAN_FUNC_INVALID"] == 0, nanf["__NL80211_NAN_FUNC_INVALID"]
	assert nanf["NL80211_NAN_FUNC_TTL"]

	srf_truth = {"__NL80211_NAN_SRF_INVALID":0,
		"NL80211_NAN_SRF_INCLUDE":1,
		"NL80211_NAN_SRF_BF":2,
		"NL80211_NAN_SRF_BF_IDX":3,
		"NL80211_NAN_SRF_MAC_ADDRS":4,
		"NUM_NL80211_NAN_SRF_ATTR":5,
		"NL80211_NAN_SRF_ATTR_MAX":4
	}

	srf = enums[1]
	print(srf[0])
	for name,value in srf_truth.items():
		print("name={} value={}".format(name, value))
		assert name in srf, name
		assert srf[name] == value, (name,value,srf[name])

def test():
	test_enum_regex()
	test_symbol_regex()
#	test_parse()

def parse(infile):
	reader = readlines(infile)
	enum_finder = enum_counter(reader)
	return [enum for enum in enum_finder]

def parse_file(infilename):
	with open(infilename, "r") as infile:
		return parse(infile)

def main():
#	return test()

	infilename = "nl80211.h"
	enums = parse_file(infilename)
	print("found len=%d enums in file=%s" % (len(enums),infilename))

	for e in enums:
		print("{} {} {}".format(e.name, len(e.values), len(e.names)))

	enum_names = {e.name: e for e in enums}
	print(enum_names.keys())
	nl80211_attrs = enum_names["nl80211_attrs"]
	mystery_number = 46
	print("num=%d enum=%s" % (mystery_number, nl80211_attrs[mystery_number]))

#	infile = open(infilename,"r")
#	infile.close()

if __name__ == '__main__':
	logging.basicConfig(level=logging.DEBUG)
	main()
