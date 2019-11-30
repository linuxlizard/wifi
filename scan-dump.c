#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>

#include "core.h"
#include "iw.h"
#include "list.h"
#include "bss.h"

static int valid_handler(struct nl_msg *msg, void *arg)
{
	struct list_head* bss_list = (struct list_head*)arg;
	struct BSS* bss;

	INFO("%s\n", __func__);

	struct nlmsghdr *hdr = nlmsg_hdr(msg);
	struct genlmsghdr* gnlh = (struct genlmsghdr*)nlmsg_data(hdr);

	struct nlattr* tb_msg[NL80211_ATTR_MAX + 1];
	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	peek_nla_attr(tb_msg, NL80211_ATTR_MAX);

	int err=0;

	if (!tb_msg[NL80211_ATTR_BSS]) {
		return NL_SKIP;
	}

//	err = 0;
	err = parse_nla_bss(tb_msg[NL80211_ATTR_BSS], &bss);
	printf( "%s %d\n", __func__, __LINE__ );
	if (err != 0) {
		goto fail;
	}
	list_add(&bss->node, bss_list);

	DBG("%s success\n", __func__);
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

	LIST_HEAD(bss_list);
	struct nl_cb* cb = nl_cb_alloc(NL_CB_DEFAULT);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, (void*)&bss_list);

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
		INFO("nl_recvmsgs err=%d\n", err);
	}

	struct BSS* bss;
	list_for_each_entry(bss, &bss_list, node) {
		XASSERT(bss->cookie == BSS_COOKIE, bss->cookie);
		INFO("%s\n", bss->bssid_str);
	}

	bss_free_list(&bss_list);
	nl_cb_put(cb);
	nl_socket_free(nl_sock);
	nlmsg_free(msg);
	return EXIT_SUCCESS;
}

