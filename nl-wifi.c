/* davep 20181011 ; learning nl80211 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <net/if.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>
#include <linux/nl80211.h>

int valid_handler(struct nl_msg *msg, void *arg)
{
	printf("%s %p %p\n", __func__, (void *)msg, arg);
	nl_msg_dump(msg,stdout);

	struct nlmsghdr *hdr = nlmsg_hdr(msg);

	int datalen = nlmsg_datalen(hdr);
	printf("datalen=%d attrlen=%d\n", datalen, nlmsg_attrlen(hdr,0));

	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];

	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	int i;
	for (i=0 ; i<NL80211_ATTR_MAX ; i++ ) {
		if (tb_msg[i]) {
			printf("%d=%p type=%d len=%d\n", i, (void *)tb_msg[i], nla_type(tb_msg[i]), nla_len(tb_msg[i]));
		}
	}

	if (tb_msg[NL80211_ATTR_WIPHY]) {
		uint32_t phy_id = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
		printf("phy_id=%u\n", phy_id);
	}

	if (tb_msg[NL80211_ATTR_IFINDEX]) {
		printf("ifindex=%d\n", nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]));
	}

	if (tb_msg[NL80211_ATTR_IFNAME]) {
		printf("ifname=%s\n", nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));
	}

	if (tb_msg[NL80211_ATTR_IFTYPE]) {
		printf("iftype\n");
	}

	if (tb_msg[NL80211_ATTR_MAC]) {
		struct nlattr *mac = tb_msg[NL80211_ATTR_MAC];
		printf("mac type=%d len=%d ok=%d\n", nla_type(mac), nla_len(mac), nla_ok(mac,0));
	}

	if (tb_msg[NL80211_ATTR_KEY_DATA]) {
		printf("keydata\n");
	}

	if (tb_msg[NL80211_ATTR_GENERATION]) {
		printf("attr generation\n");
	}

	if (tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]) {
		printf("tx_power_level\n");
	}

	if (tb_msg[NL80211_ATTR_WDEV]) {
		printf("wdev\n");
	}

	if (tb_msg[NL80211_ATTR_WIPHY_TXQ_PARAMS]) {
		printf("txq_params\n");
	}

	return NL_OK;
}


int main(void)
{
	struct nl_sock *nl_sock;
	const char *ifname = "wlp1s0";

	unsigned int ifidx = if_nametoindex(ifname);
	printf("ifidx=%u\n", ifidx);

	/* starting from iw.c nl80211_init() */
	nl_sock = nl_socket_alloc();
	printf("nl_sock=%p\n", (void *)nl_sock);

	int retcode = genl_connect(nl_sock);
	printf("genl_connect retcode=%d\n", retcode);

	int nl80211_id = genl_ctrl_resolve(nl_sock, NL80211_GENL_NAME);
	printf("nl80211_id=%d\n", nl80211_id);

	struct nl_msg *msg;
	msg = nlmsg_alloc();
	printf("msg=%p\n", (void *)msg);

	genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0, NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0);

	struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
	printf("cb=%p\n", (void *)cb);
	nl_socket_set_cb(nl_sock, cb);

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, NULL);

	nl_msg_dump(msg,stdout);

	retcode = nl_send_auto(nl_sock, msg);
	printf("retcode=%d\n", retcode);

	retcode = nl_recvmsgs_default(nl_sock);
	printf("retcode=%d\n", retcode);

	nlmsg_free(msg);
	msg = NULL;

	nl_socket_free(nl_sock);
	nl_sock = NULL;

//	NL80211_CMD_GET_WIPHY

	return EXIT_SUCCESS;
}

