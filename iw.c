#include <linux/netlink.h>
#include <linux/nl80211.h>

#include "core.h"
#include "nlnames.h"
#include "iw.h"

// from iw scan.c
static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
	[NL80211_BSS_TSF] = { .type = NLA_U64 },
	[NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
	[NL80211_BSS_BSSID] = { .type = NLA_UNSPEC },
	[NL80211_BSS_BEACON_INTERVAL] = { .type = NLA_U16 },
	[NL80211_BSS_CAPABILITY] = { .type = NLA_U16 },
	[NL80211_BSS_INFORMATION_ELEMENTS] = { .type = NLA_UNSPEC },
	[NL80211_BSS_SIGNAL_MBM] = { .type = NLA_U32 },
	[NL80211_BSS_SIGNAL_UNSPEC] = { .type = NLA_U8 },
	[NL80211_BSS_STATUS] = { .type = NLA_U32 },
	[NL80211_BSS_SEEN_MS_AGO] = { .type = NLA_U32 },
	[NL80211_BSS_BEACON_IES] = { .type = NLA_UNSPEC },
};

void peek_nla_attr( struct nlattr* tb_msg[], size_t count)
{
	for (size_t i=0 ; i<count; i++ ) {
		if (tb_msg[i]) {
			const char *name = to_string_nl80211_attrs(i);
			DBG("%s i=%zu %s type=%d len=%u\n", __func__, 
				i, name, nla_type(tb_msg[i]), nla_len(tb_msg[i]));
		}
	}
}

static void peek_nla_bss(struct nlattr* bss_msg[static NL80211_BSS_MAX], size_t count)
{
	for (size_t i=0 ; i<count; i++ ) {
		if (bss_msg[i]) {
			const char *name = to_string_nl80211_bss(i);
			DBG("%s i=%zu %s type=%d len=%u\n", __func__, 
				i, name, nla_type(bss_msg[i]), nla_len(bss_msg[i]));
		}
	}
}

int parse_nla_ies(struct nlattr* ies, struct IE_List* ie_list)
{
	int err = decode_ie_buf( nla_data(ies), nla_len(ies), ie_list);
	if (err) {
		return err;
	}

	return 0;
}

int parse_nla_bss(struct nlattr* attr)
{
	struct nlattr *bss[5];
//	struct nlattr *bss[NL80211_BSS_MAX + 1];

	if (nla_parse_nested(bss, NL80211_BSS_MAX,
			     attr,
			     bss_policy)) {
		ERR("%s failed to parse nested attributes!\n", __func__);
		return -EINVAL;
	}

	peek_nla_bss(bss, NL80211_BSS_MAX);

	struct IE_List ie_list = IE_LIST_BLANK;
	int err = ie_list_init(&ie_list);
	if (err) {
		return err;
	}

	struct nlattr *ies = bss[NL80211_BSS_INFORMATION_ELEMENTS];
	if (ies) {
		err = parse_nla_ies(ies, &ie_list);
		if (err) {
			goto fail;
		}
	}

	ie_list_peek(__func__, &ie_list);

	return 0;
fail:
	ie_list_release(&ie_list);
	return err;
}

