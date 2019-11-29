#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>

#include "core.h"
#include "iw.h"

static int valid_handler(struct nl_msg *msg, void *arg)
{
	(void)arg;

	INFO("%s\n", __func__);

	struct nlmsghdr *hdr = nlmsg_hdr(msg);
	struct genlmsghdr* gnlh = (struct genlmsghdr*)nlmsg_data(hdr);

	struct nlattr* tb_msg[NL80211_ATTR_MAX + 1];
	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	peek_nla_attr(tb_msg, NL80211_ATTR_MAX);

	int err=0;

	if (tb_msg[NL80211_ATTR_BSS]) {
		err = parse_nla_bss(tb_msg[NL80211_ATTR_BSS]);
		if (err != 0) {
			goto fail;
		}

	}

	return NL_OK;
fail:
	return NL_SKIP;
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s ifname\n", argv[0]);
		exit(1);
	}

	const char* ifname = argv[1];

	struct nl_cb* cb = nl_cb_alloc(NL_CB_DEFAULT);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, NULL);

	struct nl_sock* nl_sock = nl_socket_alloc_cb(cb);
	int err = genl_connect(nl_sock);

	int nl80211_id = genl_ctrl_resolve(nl_sock, NL80211_GENL_NAME);

	int ifidx = if_nametoindex(ifname);

	struct nl_msg* msg = nlmsg_alloc();

	genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0, 
						NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);

	nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifidx);
	err = nl_send_auto(nl_sock, msg);

	while (err > 0) {
		err = nl_recvmsgs(nl_sock, cb);
	}

	nl_cb_put(cb);
	return EXIT_SUCCESS;
}

