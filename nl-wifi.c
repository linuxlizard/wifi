/* davep 20181011 ; learning nl80211 */
//https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/nl80211.h
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <errno.h>
#include <assert.h>

#include <sys/socket.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <linux/nl80211.h>

#include "hdump.h"
#include "nlnames.h"

/* iw-4.9 iw.h */
#define BIT(x) (1ULL<<(x))

/* iw-4.9 station.c */
enum plink_state {
	LISTEN,
	OPN_SNT,
	OPN_RCVD,
	CNF_RCVD,
	ESTAB,
	HOLDING,
	BLOCKED
};

void peek(struct nl_msg *msg)
{
	struct nlmsghdr * nlh;
	struct nlattr * attr;
	int i, rem;

	// curious about how to use nlmsg_for_each_xxx()
	// tests/check-attr.c from libnl-3.2.25
	printf( "%s\n", __func__);

	nlh = nlmsg_hdr(msg);
	i = 1;
//	int len = NLMSG_LENGTH(nlh);
	int len = 1024;
	printf("peek len=%d\n", len);

	hex_dump("peek", (const unsigned char *)msg, 64);

	int hdrlen = 0;
	nlmsg_attrdata(nlh, hdrlen);
	nlmsg_attrlen(nlh, hdrlen);

	nlmsg_for_each_attr(attr, nlh, 0, rem) {
		printf("peek type=%d len=%d\n", nla_type(attr), nla_len(attr));
		i++;
	}
}

/* iw-4.9 util.c */
void mac_addr_n2a(char *mac_addr, unsigned char *arg)
{
	int i, l;

	l = 0;
	for (i = 0; i < ETH_ALEN ; i++) {
		if (i == 0) {
			sprintf(mac_addr+l, "%02x", arg[i]);
			l += 2;
		} else {
			sprintf(mac_addr+l, ":%02x", arg[i]);
			l += 3;
		}
	}
}

/* 
 * iw-4.9 station.c 
 */
void parse_bss_param(struct nlattr *bss_param_attr)
{
	struct nlattr *bss_param_info[NL80211_STA_BSS_PARAM_MAX + 1], *info;
	static struct nla_policy bss_poilcy[NL80211_STA_BSS_PARAM_MAX + 1] = {
		[NL80211_STA_BSS_PARAM_CTS_PROT] = { .type = NLA_FLAG },
		[NL80211_STA_BSS_PARAM_SHORT_PREAMBLE] = { .type = NLA_FLAG },
		[NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME] = { .type = NLA_FLAG },
		[NL80211_STA_BSS_PARAM_DTIM_PERIOD] = { .type = NLA_U8 },
		[NL80211_STA_BSS_PARAM_BEACON_INTERVAL] = { .type = NLA_U16 },
	};

	if (nla_parse_nested(bss_param_info, NL80211_STA_BSS_PARAM_MAX,
			     bss_param_attr, bss_poilcy)) {
		printf("failed to parse nested bss param attributes!");
	}

	info = bss_param_info[NL80211_STA_BSS_PARAM_DTIM_PERIOD];
	if (info)
		printf("\n\tDTIM period:\t%u", nla_get_u8(info));
	info = bss_param_info[NL80211_STA_BSS_PARAM_BEACON_INTERVAL];
	if (info)
		printf("\n\tbeacon interval:%u", nla_get_u16(info));
	info = bss_param_info[NL80211_STA_BSS_PARAM_CTS_PROT];
	if (info) {
		printf("\n\tCTS protection:");
		if (nla_get_u16(info))
			printf("\tyes");
		else
			printf("\tno");
	}
	info = bss_param_info[NL80211_STA_BSS_PARAM_SHORT_PREAMBLE];
	if (info) {
		printf("\n\tshort preamble:");
		if (nla_get_u16(info))
			printf("\tyes");
		else
			printf("\tno");
	}
	info = bss_param_info[NL80211_STA_BSS_PARAM_SHORT_SLOT_TIME];
	if (info) {
		printf("\n\tshort slot time:");
		if (nla_get_u16(info))
			printf("yes");
		else
			printf("no");
	}
}

void parse_tid_stats(struct nlattr *tid_stats_attr)
{
	struct nlattr *stats_info[NL80211_TID_STATS_MAX + 1], *tidattr, *info;
	static struct nla_policy stats_policy[NL80211_TID_STATS_MAX + 1] = {
		[NL80211_TID_STATS_RX_MSDU] = { .type = NLA_U64 },
		[NL80211_TID_STATS_TX_MSDU] = { .type = NLA_U64 },
		[NL80211_TID_STATS_TX_MSDU_RETRIES] = { .type = NLA_U64 },
		[NL80211_TID_STATS_TX_MSDU_FAILED] = { .type = NLA_U64 },
	};
	int rem, i = 0;

	printf("\n\tMSDU:\n\t\tTID\trx\ttx\ttx retries\ttx failed");
	nla_for_each_nested(tidattr, tid_stats_attr, rem) {
		if (nla_parse_nested(stats_info, NL80211_TID_STATS_MAX,
				     tidattr, stats_policy)) {
			printf("failed to parse nested stats attributes!");
			return;
		}
		printf("\n\t\t%d", i++);
		info = stats_info[NL80211_TID_STATS_RX_MSDU];
		if (info)
			printf("\t%llu", (unsigned long long)nla_get_u64(info));
		info = stats_info[NL80211_TID_STATS_TX_MSDU];
		if (info)
			printf("\t%llu", (unsigned long long)nla_get_u64(info));
		info = stats_info[NL80211_TID_STATS_TX_MSDU_RETRIES];
		if (info)
			printf("\t%llu", (unsigned long long)nla_get_u64(info));
		info = stats_info[NL80211_TID_STATS_TX_MSDU_FAILED];
		if (info)
			printf("\t\t%llu", (unsigned long long)nla_get_u64(info));
	}
}

static void print_power_mode(struct nlattr *a)
{
	enum nl80211_mesh_power_mode pm = nla_get_u32(a);

	switch (pm) {
	case NL80211_MESH_POWER_ACTIVE:
		printf("ACTIVE");
		break;
	case NL80211_MESH_POWER_LIGHT_SLEEP:
		printf("LIGHT SLEEP");
		break;
	case NL80211_MESH_POWER_DEEP_SLEEP:
		printf("DEEP SLEEP");
		break;
	default:
		printf("UNKNOWN");
		break;
	}
}

void parse_bitrate(struct nlattr *bitrate_attr, char *buf, int buflen)
{
	int rate = 0;
	char *pos = buf;
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
		[NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
		[NL80211_RATE_INFO_BITRATE32] = { .type = NLA_U32 },
		[NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
		[NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
	};

	if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
			     bitrate_attr, rate_policy)) {
		snprintf(buf, buflen, "failed to parse nested rate attributes!");
		return;
	}

	if (rinfo[NL80211_RATE_INFO_BITRATE32])
		rate = nla_get_u32(rinfo[NL80211_RATE_INFO_BITRATE32]);
	else if (rinfo[NL80211_RATE_INFO_BITRATE])
		rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
	if (rate > 0)
		pos += snprintf(pos, buflen - (pos - buf),
				"%d.%d MBit/s", rate / 10, rate % 10);

	if (rinfo[NL80211_RATE_INFO_MCS])
		pos += snprintf(pos, buflen - (pos - buf),
				" MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]));
	if (rinfo[NL80211_RATE_INFO_VHT_MCS])
		pos += snprintf(pos, buflen - (pos - buf),
				" VHT-MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_MCS]));
	if (rinfo[NL80211_RATE_INFO_40_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 40MHz");
	if (rinfo[NL80211_RATE_INFO_80_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 80MHz");
	if (rinfo[NL80211_RATE_INFO_80P80_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 80P80MHz");
	if (rinfo[NL80211_RATE_INFO_160_MHZ_WIDTH])
		pos += snprintf(pos, buflen - (pos - buf), " 160MHz");
	if (rinfo[NL80211_RATE_INFO_SHORT_GI])
		pos += snprintf(pos, buflen - (pos - buf), " short GI");
	if (rinfo[NL80211_RATE_INFO_VHT_NSS])
		pos += snprintf(pos, buflen - (pos - buf),
				" VHT-NSS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_VHT_NSS]));
}

static char *get_chain_signal(struct nlattr *attr_list)
{
	struct nlattr *attr;
	static char buf[64];
	char *cur = buf;
	int i = 0, rem;
	const char *prefix;

	if (!attr_list)
		return "";

	nla_for_each_nested(attr, attr_list, rem) {
		if (i++ > 0)
			prefix = ", ";
		else
			prefix = "[";

		cur += snprintf(cur, sizeof(buf) - (cur - buf), "%s%d", prefix,
				(int8_t) nla_get_u8(attr));
	}

	if (i)
		snprintf(cur, sizeof(buf) - (cur - buf), "] ");

	return buf;
}

static int print_sta_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	char mac_addr[20], state_name[10], dev[20];
	struct nl80211_sta_flag_update *sta_flags;
	static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
		[NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_BYTES64] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_BYTES64] = { .type = NLA_U64 },
		[NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_BEACON_RX] = { .type = NLA_U64},
		[NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
		[NL80211_STA_INFO_T_OFFSET] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_RX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_LLID] = { .type = NLA_U16 },
		[NL80211_STA_INFO_PLID] = { .type = NLA_U16 },
		[NL80211_STA_INFO_PLINK_STATE] = { .type = NLA_U8 },
		[NL80211_STA_INFO_TX_RETRIES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_FAILED] = { .type = NLA_U32 },
		[NL80211_STA_INFO_BEACON_LOSS] = { .type = NLA_U32},
		[NL80211_STA_INFO_RX_DROP_MISC] = { .type = NLA_U64},
		[NL80211_STA_INFO_STA_FLAGS] =
			{ .minlen = sizeof(struct nl80211_sta_flag_update) },
		[NL80211_STA_INFO_LOCAL_PM] = { .type = NLA_U32},
		[NL80211_STA_INFO_PEER_PM] = { .type = NLA_U32},
		[NL80211_STA_INFO_NONPEER_PM] = { .type = NLA_U32},
		[NL80211_STA_INFO_CHAIN_SIGNAL] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_CHAIN_SIGNAL_AVG] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_TID_STATS] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_BSS_PARAM] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_RX_DURATION] = { .type = NLA_U64 },
	};
	char *chain;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	/*
	 * TODO: validate the interface and mac address!
	 * Otherwise, there's a race condition as soon as
	 * the kernel starts sending station notifications.
	 */

	if (!tb[NL80211_ATTR_STA_INFO]) {
		fprintf(stderr, "sta stats missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
			     tb[NL80211_ATTR_STA_INFO],
			     stats_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	mac_addr_n2a(mac_addr, nla_data(tb[NL80211_ATTR_MAC]));
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
	printf("Station %s (on %s)", mac_addr, dev);

	if (sinfo[NL80211_STA_INFO_INACTIVE_TIME])
		printf("\n\tinactive time:\t%u ms",
			nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]));
	if (sinfo[NL80211_STA_INFO_RX_BYTES64])
		printf("\n\trx bytes:\t%llu",
		       (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_RX_BYTES64]));
	else if (sinfo[NL80211_STA_INFO_RX_BYTES])
		printf("\n\trx bytes:\t%u",
		       nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]));
	if (sinfo[NL80211_STA_INFO_RX_PACKETS])
		printf("\n\trx packets:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]));
	if (sinfo[NL80211_STA_INFO_TX_BYTES64])
		printf("\n\ttx bytes:\t%llu",
		       (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_TX_BYTES64]));
	else if (sinfo[NL80211_STA_INFO_TX_BYTES])
		printf("\n\ttx bytes:\t%u",
		       nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]));
	if (sinfo[NL80211_STA_INFO_TX_PACKETS])
		printf("\n\ttx packets:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]));
	if (sinfo[NL80211_STA_INFO_TX_RETRIES])
		printf("\n\ttx retries:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_RETRIES]));
	if (sinfo[NL80211_STA_INFO_TX_FAILED])
		printf("\n\ttx failed:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_FAILED]));
	if (sinfo[NL80211_STA_INFO_BEACON_LOSS])
		printf("\n\tbeacon loss:\t%u",
		       nla_get_u32(sinfo[NL80211_STA_INFO_BEACON_LOSS]));
	if (sinfo[NL80211_STA_INFO_BEACON_RX])
		printf("\n\tbeacon rx:\t%llu",
		       (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_BEACON_RX]));
	if (sinfo[NL80211_STA_INFO_RX_DROP_MISC])
		printf("\n\trx drop misc:\t%llu",
		       (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_RX_DROP_MISC]));

	chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL]);
	if (sinfo[NL80211_STA_INFO_SIGNAL])
		printf("\n\tsignal:  \t%d %sdBm",
			(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]),
			chain);

	chain = get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL_AVG]);
	if (sinfo[NL80211_STA_INFO_SIGNAL_AVG])
		printf("\n\tsignal avg:\t%d %sdBm",
			(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL_AVG]),
			chain);

	if (sinfo[NL80211_STA_INFO_BEACON_SIGNAL_AVG])
		printf("\n\tbeacon signal avg:\t%d dBm",
		       nla_get_u8(sinfo[NL80211_STA_INFO_BEACON_SIGNAL_AVG]));
	if (sinfo[NL80211_STA_INFO_T_OFFSET])
		printf("\n\tToffset:\t%llu us",
		       (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_T_OFFSET]));

	if (sinfo[NL80211_STA_INFO_TX_BITRATE]) {
		char buf[100];

		parse_bitrate(sinfo[NL80211_STA_INFO_TX_BITRATE], buf, sizeof(buf));
		printf("\n\ttx bitrate:\t%s", buf);
	}

	if (sinfo[NL80211_STA_INFO_RX_BITRATE]) {
		char buf[100];

		parse_bitrate(sinfo[NL80211_STA_INFO_RX_BITRATE], buf, sizeof(buf));
		printf("\n\trx bitrate:\t%s", buf);
	}

	if (sinfo[NL80211_STA_INFO_RX_DURATION])
		printf("\n\trx duration:\t%lld us",
		       (unsigned long long)nla_get_u64(sinfo[NL80211_STA_INFO_RX_DURATION]));

	if (sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]) {
		uint32_t thr;

		thr = nla_get_u32(sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]);
		/* convert in Mbps but scale by 1000 to save kbps units */
		thr = thr * 1000 / 1024;

		printf("\n\texpected throughput:\t%u.%uMbps",
		       thr / 1000, thr % 1000);
	}

	if (sinfo[NL80211_STA_INFO_LLID])
		printf("\n\tmesh llid:\t%d",
			nla_get_u16(sinfo[NL80211_STA_INFO_LLID]));
	if (sinfo[NL80211_STA_INFO_PLID])
		printf("\n\tmesh plid:\t%d",
			nla_get_u16(sinfo[NL80211_STA_INFO_PLID]));
	if (sinfo[NL80211_STA_INFO_PLINK_STATE]) {
		switch (nla_get_u8(sinfo[NL80211_STA_INFO_PLINK_STATE])) {
		case LISTEN:
			strcpy(state_name, "LISTEN");
			break;
		case OPN_SNT:
			strcpy(state_name, "OPN_SNT");
			break;
		case OPN_RCVD:
			strcpy(state_name, "OPN_RCVD");
			break;
		case CNF_RCVD:
			strcpy(state_name, "CNF_RCVD");
			break;
		case ESTAB:
			strcpy(state_name, "ESTAB");
			break;
		case HOLDING:
			strcpy(state_name, "HOLDING");
			break;
		case BLOCKED:
			strcpy(state_name, "BLOCKED");
			break;
		default:
			strcpy(state_name, "UNKNOWN");
			break;
		}
		printf("\n\tmesh plink:\t%s", state_name);
	}
	if (sinfo[NL80211_STA_INFO_LOCAL_PM]) {
		printf("\n\tmesh local PS mode:\t");
		print_power_mode(sinfo[NL80211_STA_INFO_LOCAL_PM]);
	}
	if (sinfo[NL80211_STA_INFO_PEER_PM]) {
		printf("\n\tmesh peer PS mode:\t");
		print_power_mode(sinfo[NL80211_STA_INFO_PEER_PM]);
	}
	if (sinfo[NL80211_STA_INFO_NONPEER_PM]) {
		printf("\n\tmesh non-peer PS mode:\t");
		print_power_mode(sinfo[NL80211_STA_INFO_NONPEER_PM]);
	}

	if (sinfo[NL80211_STA_INFO_STA_FLAGS]) {
		sta_flags = (struct nl80211_sta_flag_update *)
			    nla_data(sinfo[NL80211_STA_INFO_STA_FLAGS]);

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
			printf("\n\tauthorized:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_AUTHORIZED))
				printf("yes");
			else
				printf("no");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_AUTHENTICATED)) {
			printf("\n\tauthenticated:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_AUTHENTICATED))
				printf("yes");
			else
				printf("no");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_ASSOCIATED)) {
			printf("\n\tassociated:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_ASSOCIATED))
				printf("yes");
			else
				printf("no");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE)) {
			printf("\n\tpreamble:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_SHORT_PREAMBLE))
				printf("short");
			else
				printf("long");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_WME)) {
			printf("\n\tWMM/WME:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_WME))
				printf("yes");
			else
				printf("no");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_MFP)) {
			printf("\n\tMFP:\t\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_MFP))
				printf("yes");
			else
				printf("no");
		}

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_TDLS_PEER)) {
			printf("\n\tTDLS peer:\t");
			if (sta_flags->set & BIT(NL80211_STA_FLAG_TDLS_PEER))
				printf("yes");
			else
				printf("no");
		}
	}

	if (sinfo[NL80211_STA_INFO_TID_STATS] && arg != NULL &&
	    !strcmp((char *)arg, "-v"))
		parse_tid_stats(sinfo[NL80211_STA_INFO_TID_STATS]);
	if (sinfo[NL80211_STA_INFO_BSS_PARAM])
		parse_bss_param(sinfo[NL80211_STA_INFO_BSS_PARAM]);
	if (sinfo[NL80211_STA_INFO_CONNECTED_TIME])
		printf("\n\tconnected time:\t%u seconds",
			nla_get_u32(sinfo[NL80211_STA_INFO_CONNECTED_TIME]));

	printf("\n");
	return NL_SKIP;
}
/*
 * end nl-4.9 
 */

void decode_attr_bss( struct nlattr *attr )
{
	/* from iw-4.9 */
	struct nlattr *bss[NL80211_BSS_MAX + 1];
	static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
		[NL80211_BSS_TSF] = { .type = NLA_U64 },
		[NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
		[NL80211_BSS_BSSID] = { },
		[NL80211_BSS_BEACON_INTERVAL] = { .type = NLA_U16 },
		[NL80211_BSS_CAPABILITY] = { .type = NLA_U16 },
		[NL80211_BSS_INFORMATION_ELEMENTS] = { },
		[NL80211_BSS_SIGNAL_MBM] = { .type = NLA_U32 },
		[NL80211_BSS_SIGNAL_UNSPEC] = { .type = NLA_U8 },
		[NL80211_BSS_STATUS] = { .type = NLA_U32 },
	};

	/* decode ATTR_BSS */
	printf("%s len=%" PRIu16 " type=%" PRIu16 "\n", __func__, attr->nla_len, attr->nla_type);

	/* from iw-4.9 */
	if (nla_parse_nested(bss, NL80211_BSS_MAX,
			     attr, bss_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return;
	}

	// report attrs not handled in my crappy code below
	ssize_t counter=0;

	enum nl80211_bss bss_attr;
	for (int i=0 ; i<NL80211_BSS_MAX ; i++ ) {
		if (bss[i]) {
			bss_attr = nla_type(bss[i]);
			printf("%s %d=%p type=%d len=%d\n", __func__, i, (void *)bss[i], bss_attr, nla_len(bss[i]));
			counter++;
		}
	}

	if (bss[NL80211_BSS_BSSID]) {
		counter--;
		char mac_addr[64];
		mac_addr_n2a(mac_addr, nla_data(bss[NL80211_BSS_BSSID]));
		printf("bssid=%s\n", mac_addr);
	}

	if (bss[NL80211_BSS_FREQUENCY]) {
		counter--;
		uint32_t freq = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
		printf("freq=%" PRIu32 "\n", freq);
	}

	if (bss[NL80211_BSS_TSF]) {
		counter--;
		/* Timing synchronization function 
		 * https://en.wikipedia.org/wiki/Timing_synchronization_function 
		 */
		uint64_t tsf = nla_get_u64(bss[NL80211_BSS_TSF]);
		printf("tsf=%#" PRIx64 "\n", tsf);
	}

	if (bss[NL80211_BSS_BEACON_INTERVAL]) {
		counter--;
		uint16_t bi = nla_get_u16(bss[NL80211_BSS_BEACON_INTERVAL]);
		printf("beacon interval=%" PRIu16 "\n", bi);
	}

	if (bss[NL80211_BSS_CAPABILITY]) {
		counter--;
		uint16_t capa = nla_get_u16(bss[NL80211_BSS_CAPABILITY]);
		printf("capabilities=%#" PRIx16 "\n", capa);
	}

	if (bss[NL80211_BSS_INFORMATION_ELEMENTS]) {
		counter--;
		uint8_t *ie = nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
		size_t ie_len = nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]);
		printf("found len=%zu of information elements\n", ie_len);
		hex_dump("ie", ie, ie_len);
	}

	if (bss[NL80211_BSS_SIGNAL_MBM]) {
		counter--;
		int32_t mbm = (int32_t)nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]);
		printf("mbm=%" PRIi32 "\n", mbm);
	}

	if (bss[NL80211_BSS_SIGNAL_UNSPEC]) {
		counter--;
		uint8_t signal = nla_get_u8(bss[NL80211_BSS_SIGNAL_UNSPEC]);
		printf("signal=%" PRIu8 "\n", signal);
	}

	if (bss[NL80211_BSS_STATUS]) {
		counter--;
		uint32_t status = nla_get_u32(bss[NL80211_BSS_STATUS]);
		printf("status=%" PRIu32 "\n", status);
	}

	if (bss[NL80211_BSS_SEEN_MS_AGO]) {
		counter--;
		uint32_t seen_ms = nla_get_u32(bss[NL80211_BSS_SEEN_MS_AGO]);
		printf("seen %" PRIu32 " ms ago\n", seen_ms);
	}

	if (bss[NL80211_BSS_BEACON_IES]) {
		counter--;
		uint8_t *beacon_ie = nla_data(bss[NL80211_BSS_BEACON_IES]);
		hex_dump("beacon_ie", beacon_ie, nla_len(bss[NL80211_BSS_BEACON_IES]));
	}

	if (bss[NL80211_BSS_CHAN_WIDTH]) {
		counter--;
		enum nl80211_bss_scan_width chan_width = nla_get_u32(bss[NL80211_BSS_CHAN_WIDTH]);
		printf("chan width=%" PRIu32 "\n", chan_width);
	}

	if (bss[NL80211_BSS_BEACON_TSF]) {
		counter --;
		uint64_t beacon_tsf = nla_get_u64(bss[NL80211_BSS_BEACON_TSF]);
		printf("beacon tsf=%" PRIu64 "\n", beacon_tsf);
	}

	if (bss[NL80211_BSS_PRESP_DATA]) {
		counter--;
		bool presp_data = nla_get_flag(bss[NL80211_BSS_PRESP_DATA]);
		printf("presp_data is %s\n", presp_data ? "true": "false");
	}

	if (bss[NL80211_BSS_LAST_SEEN_BOOTTIME]) {
		counter--;
		/* nanoseconds */
		uint64_t last_seen_boottime = nla_get_u64(bss[NL80211_BSS_LAST_SEEN_BOOTTIME]);
		printf("last seen boottime=%" PRIu64 "ns\n", last_seen_boottime);
	}




	printf("%s counter=%zd unhandled attributes\n", __func__, counter);
}

int valid_handler(struct nl_msg *msg, void *arg)
{
	printf("%s %p %p\n", __func__, (void *)msg, arg);

//	hex_dump("msg", (const unsigned char *)msg, 128);

//	nl_msg_dump(msg,stdout);

	struct nlmsghdr *hdr = nlmsg_hdr(msg);

	int datalen = nlmsg_datalen(hdr);
	printf("datalen=%d attrlen=%d\n", datalen, nlmsg_attrlen(hdr,0));

	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];

	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	printf("%s cmd=%d\n", __func__, (int)(gnlh->cmd));

	// report attrs not handled in my crappy code below
	ssize_t counter=0;

	for (int i=0 ; i<NL80211_ATTR_MAX ; i++ ) {
		if (tb_msg[i]) {
			printf("%s %d=%p type=%d len=%d\n", __func__, i, (void *)tb_msg[i], nla_type(tb_msg[i]), nla_len(tb_msg[i]));
			counter++;
		}
	}

	if (tb_msg[NL80211_ATTR_WIPHY]) {
		counter--;
		uint32_t phy_id = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
		printf("phy_id=%u\n", phy_id);
	}

	if (tb_msg[NL80211_ATTR_WIPHY_NAME]) {
		counter--;
		const char *p = nla_get_string(tb_msg[NL80211_ATTR_WIPHY_NAME]);
		printf("phy_name=%s\n", p);
	}

	if (tb_msg[NL80211_ATTR_IFINDEX]) {
		counter--;
		printf("ifindex=%d\n", nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]));
	}

	if (tb_msg[NL80211_ATTR_IFNAME]) {
		counter--;
		printf("ifname=%s\n", nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));
	}

	if (tb_msg[NL80211_ATTR_IFTYPE]) {
		counter--;
		struct nlattr *attr = tb_msg[NL80211_ATTR_IFTYPE];
		enum nl80211_iftype * iftype = nla_data(attr);
		printf("iftype=%d\n", *iftype);
	}

	if (tb_msg[NL80211_ATTR_MAC]) {
		counter--;
		struct nlattr *attr = tb_msg[NL80211_ATTR_MAC];
		printf("attr type=%d len=%d ok=%d\n", nla_type(attr), nla_len(attr), nla_ok(attr,0));
		uint8_t *mac = nla_data(attr);
		hex_dump("mac", mac, nla_len(attr));
	}

	if (tb_msg[NL80211_ATTR_KEY_DATA]) {
		counter--;
		printf("keydata=?\n");
	}

	if (tb_msg[NL80211_ATTR_GENERATION]) {
		counter--;
		uint32_t attr_gen = nla_get_u32(tb_msg[NL80211_ATTR_GENERATION]);
		printf("attr generation=%#" PRIx32 "\n", attr_gen);
	}

	if (tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]) {
		counter--;
		uint32_t tx_power = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]);
		printf("tx_power_level=%" PRIu32 "\n", tx_power);
		// printf taken from iw-4.14 interface.c print_iface_handler()
		printf("tx_power %d.%.2d dBm\n", tx_power / 100, tx_power % 100);
	}

	if (tb_msg[NL80211_ATTR_WDEV]) {
		counter--;
		uint64_t wdev = nla_get_u64(tb_msg[NL80211_ATTR_WDEV]);
		printf("wdev=%#" PRIx64 "\n", wdev);
	}

	if (tb_msg[NL80211_ATTR_CHANNEL_WIDTH]) {
		counter--;
		enum nl80211_chan_width w = nla_get_u32(tb_msg[NL80211_ATTR_CHANNEL_WIDTH]);
		printf("channel_width=%" PRIu32 "\n", w);
	}

	if (tb_msg[NL80211_ATTR_WIPHY_CHANNEL_TYPE]) {
		counter--;
		printf("channel_type=?\n");
	}

	if (tb_msg[NL80211_ATTR_WIPHY_FREQ]) {
		counter--;
		uint32_t wiphy_freq = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_FREQ]);
		printf("wiphy_freq=%" PRIu32 "\n", wiphy_freq);
	}

	if (tb_msg[NL80211_ATTR_CENTER_FREQ1]) {
		counter--;
		uint32_t center_freq1 = nla_get_u32(tb_msg[NL80211_ATTR_CENTER_FREQ1]);
		printf("center_freq1=%" PRIu32 "\n", center_freq1);
	}

	if (tb_msg[NL80211_ATTR_CENTER_FREQ2]) {
		counter--;
		uint32_t center_freq2 = nla_get_u32(tb_msg[NL80211_ATTR_CENTER_FREQ2]);
		printf("center_freq2=%" PRIu32 "\n", center_freq2);
	}

	if (tb_msg[NL80211_ATTR_WIPHY_TXQ_PARAMS]) {
		counter--;
		printf("txq_params=?\n");
	}

	if (tb_msg[NL80211_ATTR_STA_INFO]) {
		counter--;
		/* @NL80211_ATTR_STA_INFO: information about a station, part of station info
		 *  given for %NL80211_CMD_GET_STATION, nested attribute containing
		 *  info as possible, see &enum nl80211_sta_info.
		 */
		printf("sta info=?\n");
		print_sta_handler(msg, arg);
	}

	if (tb_msg[NL80211_ATTR_BANDS]) {
		counter--;
		/* @NL80211_ATTR_BANDS: operating bands configuration.  This is a u32
		 *	bitmask of BIT(NL80211_BAND_*) as described in %enum
		 *	nl80211_band.  For instance, for NL80211_BAND_2GHZ, bit 0
		 *	would be set.  This attribute is used with
		 *	%NL80211_CMD_START_NAN and %NL80211_CMD_CHANGE_NAN_CONFIG, and
		 *	it is optional.  If no bands are set, it means don't-care and
		 *	the device will decide what to use.
		 */

		enum nl80211_band_attr band = nla_get_u32(tb_msg[NL80211_ATTR_BANDS]);
		// results are kinda boring ... 
		printf("attr_bands=%#" PRIx32 "\n", band);
	}

	if (tb_msg[NL80211_ATTR_BSS]) {
		counter--;
		printf("bss=?\n");
		decode_attr_bss(tb_msg[NL80211_ATTR_BSS]);
	}

	if (tb_msg[NL80211_ATTR_SCAN_FREQUENCIES]) {
		counter--;
		printf("scan_frequencies\n");
		struct nlattr *nst;
		int rem_nst;
		nla_for_each_nested(nst, tb_msg[NL80211_ATTR_SCAN_FREQUENCIES], rem_nst)
			printf(" %d", nla_get_u32(nst));
		printf(",");
	}

	if (tb_msg[NL80211_ATTR_SCAN_SSIDS]) {
		counter--;
		printf("scan_ssids\n");
	}

	printf("%s counter=%zd unhandled attributes\n", __func__, counter);

	return NL_SKIP;
//	return NL_OK;
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg)
{
	printf("%s\n", __func__);
	(void)nla;
	(void)err;
	(void)arg;

	assert(0);
	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	printf("%s\n", __func__);
	(void)msg;

	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
	printf("%s\n", __func__);
	(void)msg;

	int *ret = arg;
	*ret = 0;
	return NL_STOP;
}

#if 0
static int msg_in_handler(struct nl_msg *msg, void *arg )
{
	printf("%s\n", __func__);
	/* "Called for every message received." */
	(void)msg;
	(void)arg;
	return NL_OK;
}
#endif

/* from iw genl.c */
struct handler_args {
	const char *group;
	int id;
};

/* from iw genl.c */
static int family_handler(struct nl_msg *msg, void *arg)
{
	struct handler_args *grp = arg;
	struct nlattr *tb[CTRL_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *mcgrp;
	int rem_mcgrp;

	printf("%s\n", __func__);
	nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[CTRL_ATTR_MCAST_GROUPS])
		return NL_SKIP;

	nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], rem_mcgrp) {
		struct nlattr *tb_mcgrp[CTRL_ATTR_MCAST_GRP_MAX + 1];

		nla_parse(tb_mcgrp, CTRL_ATTR_MCAST_GRP_MAX,
			  nla_data(mcgrp), nla_len(mcgrp), NULL);

		if (!tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME] ||
		    !tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID])
			continue;
		if (strncmp(nla_data(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]),
			    grp->group, nla_len(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME])))
			continue;
		grp->id = nla_get_u32(tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]);
		break;
	}

	return NL_SKIP;
}

/* from iw genl.c but I renamed from nl_get_multicast_id() because it was
 * confusing me thinking it was part of the libnl. 
 */
int get_multicast_id(struct nl_sock *sock, const char *family, const char *group)
{
	struct nl_msg *msg;
	struct nl_cb *cb;
	int ret, ctrlid;
	struct handler_args grp = {
		.group = group,
		.id = -ENOENT,
	};

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb) {
		ret = -ENOMEM;
		goto out_fail_cb;
	}

	ctrlid = genl_ctrl_resolve(sock, "nlctrl");

	genlmsg_put(msg, 0, 0, ctrlid, 0,
		    0, CTRL_CMD_GETFAMILY, 0);

	ret = -ENOBUFS;
	NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

	ret = nl_send_auto_complete(sock, msg);
	if (ret < 0)
		goto out;

	ret = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &ret);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &ret);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, family_handler, &grp);

	while (ret > 0)
		nl_recvmsgs(sock, cb);

	if (ret == 0)
		ret = grp.id;
 nla_put_failure:
 out:
	nl_cb_put(cb);
 out_fail_cb:
	nlmsg_free(msg);
	return ret;
}

/* from iw event.c */
static int no_seq_check(struct nl_msg *msg, void *arg)
{
	(void)msg;
	(void)arg;

	return NL_OK;
}

int event_listener(void)
{
	struct nl_sock* nl_sock = nl_socket_alloc();
	if (!nl_sock) {
		goto leave;
	}

	int err = genl_connect(nl_sock);
	if (err < 0) {
		fprintf(stderr,"genl_connect failed err=%d\n", err);
		goto leave;
	}

	struct nl_cb *cb = nl_cb_alloc(NL_CB_DEBUG);
	if (!cb) {
		fprintf(stderr,"nl_cb_alloc() failed\n");
		goto leave;
	}

	int mcid = get_multicast_id(nl_sock, "nl80211", "scan");
	if (mcid < 0) {
		fprintf(stderr, "get_multicast_id err=%d \"%s\"\n", mcid, strerror(mcid));
		goto leave;
	}
	printf("multicast id=%d\n", mcid);

	nl_socket_add_membership(nl_sock, mcid);

	/* from iw event.c */
	/* no sequence checking for multicast messages */
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, NULL);

	while (1) { 
		printf("calling nl_recvmsgs...\n");
		err = nl_recvmsgs(nl_sock, cb);
		if (err < 0) {
			fprintf(stderr, "nl_recvmsgs err=%d \"%s\"\n", err, strerror(errno));
			break;
		}

	}

leave:
	if (nl_sock) {
		nl_socket_free(nl_sock);
		nl_sock = NULL;
	}
	if (cb) {
		nl_cb_put(cb);
	}

	return 0;
}

int main(void)
{
	const char *ifname = "wlp1s0";

	/* learning event listening */
	return event_listener();

	struct nl_sock *nl_sock;

	unsigned int ifidx = if_nametoindex(ifname);
	printf("ifidx=%u\n", ifidx);

	/* starting from iw.c nl80211_init() */
	nl_sock = nl_socket_alloc();
	printf("nl_sock=%p\n", (void *)nl_sock);

	int retcode = genl_connect(nl_sock);
	printf("genl_connect retcode=%d\n", retcode);

	/* from iw.c */
	nl_socket_set_buffer_size(nl_sock, 8192, 8192);

	int nl80211_id = genl_ctrl_resolve(nl_sock, NL80211_GENL_NAME);
	if (nl80211_id < 0) {
		exit(EXIT_FAILURE);
	}

	printf("nl80211_id=%d\n", nl80211_id);

	struct nl_cb *cb = nl_cb_alloc(NL_CB_DEBUG);
	struct nl_cb *s_cb = nl_cb_alloc(NL_CB_DEBUG);

	/* davep 20190228 ; copied from iw.c */
	int err=1;
	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, NULL);

	nl_socket_set_cb(nl_sock, s_cb);

	struct nl_msg *msg;
	msg = nlmsg_alloc();

	/* @NL80211_CMD_GET_INTERFACE: Request an interface's configuration;
	 * either a dump request for all interfaces or a specific get with a
	 * single %NL80211_ATTR_IFINDEX is supported.
	 */
	void *p = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0, 
						NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0);
	if (!p) {
		/* "Returns Pointer to user header or NULL if an error occurred." */
		fprintf(stderr, "genlmsg_put failed somehow\n");
		goto leave;
	}

	nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifidx);
	nl_msg_dump(msg,stdout);

	int arg = NL80211_CMD_GET_INTERFACE;
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, (void *)&arg );

	retcode = nl_send_auto(nl_sock, msg);
	printf("nl_send_auto retcode=%d\n", retcode);

	while (err > 0) {
		printf("calling GET_INTERFACE nl_recvmsgs_default...\n");
		retcode = nl_recvmsgs(nl_sock, cb);
		printf("GET_INTERFACE nl_recvmsgs_default retcode=%d err=%d\n", retcode, err);
	}
	err = 1;

	nlmsg_free(msg);
	msg = NULL;


	/* how about we get really daring and do a 2nd message (Oooo! Ahhh!) */
	msg = nlmsg_alloc();

	p = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0, 
						NLM_F_DUMP, NL80211_CMD_GET_STATION, 0);
	if (!p) {
		/* "Returns Pointer to user header or NULL if an error occurred." */
		fprintf(stderr, "genlmsg_put failed somehow\n");
		goto leave;
	}

	nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifidx);
	nl_msg_dump(msg,stdout);

	arg = NL80211_CMD_GET_STATION;
//	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, (void *)&arg );

	retcode = nl_send_auto(nl_sock, msg);
	printf("retcode=%d\n", retcode);

	while (err > 0) {
		printf("calling GET_STATION nl_recvmsgs_default...\n");
		retcode = nl_recvmsgs(nl_sock, cb);
		printf("GET_STATION nl_recvmsgs_default retcode=%d err=%d\n", retcode, err);
	}
	err = 1;

	nlmsg_free(msg);
	msg = NULL;


	msg = nlmsg_alloc();
	printf("msg=%p\n", (void *)msg);

	arg = NL80211_CMD_GET_WIPHY;
	p = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0, 
						NLM_F_DUMP, NL80211_CMD_GET_WIPHY, 0);
	if (!p) {
		/* "Returns Pointer to user header or NULL if an error occurred." */
		fprintf(stderr, "genlmsg_put failed somehow\n");
		goto leave;
	}

	nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifidx);
	retcode = nl_send_auto(nl_sock, msg);
	printf("retcode=%d\n", retcode);

	while (err > 0) {
		printf("GET_WIPHY calling nl_recvmsgs_default...\n");
		retcode = nl_recvmsgs(nl_sock, cb);
		printf("GET_WIPHY nl_recvmsgs_default retcode=%d err=%d\n", retcode, err);
	}
	err = 1;

	nlmsg_free(msg);
	msg = NULL;


	/* SCAN -- THE BIG KAHOONA! */
	msg = nlmsg_alloc();

	arg = NL80211_CMD_GET_SCAN;
	p = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0, 
						NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);
	if (!p) {
		/* "Returns Pointer to user header or NULL if an error occurred." */
		fprintf(stderr, "genlmsg_put failed somehow\n");
		goto leave;
	}

	nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifidx);
	retcode = nl_send_auto(nl_sock, msg);
	printf("retcode=%d\n", retcode);

	while (err > 0) {
		printf("GET_SCAN calling nl_recvmsgs_default...\n");
		retcode = nl_recvmsgs(nl_sock, cb);
		printf("GET_SCAN retcode=%d err=%d\n", retcode, err);
	}

	nlmsg_free(msg);
	msg = NULL;


leave:
	if (msg) {
		nlmsg_free(msg);
		msg = NULL;
	}

	nl_socket_free(nl_sock);
	nl_sock = NULL;

	nl_cb_put(cb);

//	NL80211_CMD_GET_WIPHY

	return EXIT_SUCCESS;
}

