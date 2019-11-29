#ifndef IW_H
#define IW_H

#include <stddef.h>
#include <linux/netlink.h>
#include <netlink/attr.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>

void peek_nla_attr( struct nlattr* tb_msg[], size_t count);
void peek_nla_bss(struct nlattr* bss_msg[], size_t count);

int parse_nla_bss(struct nlattr* attr);

#endif

