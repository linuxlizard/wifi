#include <string.h>

#include "core.h"
#include "bss.h"

struct BSS* bss_new(macaddr bssid)
{
	struct BSS* bss;

	bss = calloc(1, sizeof(struct BSS));
	if (!bss) {
		return NULL;
	}

	bss->cookie = BSS_COOKIE;
//	INIT_LIST_HEAD(&bss->node);
	memcpy(bss->bssid, bssid, ETH_ALEN);

	int err = ie_list_init(&bss->ie_list);
	if (err) {
		PTR_FREE(bss);
		return NULL;
	}

	return bss;
}

void bss_free(struct BSS** pbss)
{
	struct BSS* bss;

	PTR_ASSIGN(bss, *pbss);

	ie_list_release(&bss->ie_list);

	memset(bss, POISON, sizeof(struct BSS));
	PTR_FREE(bss);
}

void bss_free_list(struct list_head* bss_list)
{
	while( !list_empty(bss_list)) {
		struct BSS* bss = list_first_entry(bss_list, typeof(*bss), node);
		if (!bss) {
			break;
		}

		XASSERT(bss->cookie == BSS_COOKIE, bss->cookie);
		list_del(&bss->node);
		XASSERT(bss->cookie == BSS_COOKIE, bss->cookie);
		bss_free(&bss);
	}
}

