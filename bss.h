#ifndef BSS_H
#define BSS_H

#include <linux/if_ether.h>
#include <linux/nl80211.h>

#include "list.h"
#include "ie.h"

#define BSS_COOKIE 0xc36194b7

typedef uint8_t macaddr[ETH_ALEN];

struct BSS 
{
	uint32_t cookie;
	struct list_head node;
	macaddr bssid;
	char bssid_str[24];

	struct IE_List ie_list;
};

struct BSS* bss_new(macaddr bssid);
void bss_free(struct BSS** pbss);

void bss_free_list(struct list_head* list);

#endif

