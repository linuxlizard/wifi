#include "core.h"
#include "bss.h"

int main(void)
{
	struct BSS* bss;

	macaddr me = { 0x00, 0x40, 0x68, 0x12, 0x34, 0x56 };
	LIST_HEAD(bss_list);

	bss = bss_new(me);
	list_add(&bss->node, &bss_list);
	size_t off = offsetof(struct BSS, node);
	DBG("off=%zu\n", off);
	DBG("%p %p %p %p\n", (void *)bss, (void *)&bss->node, (void *)bss_list.next, (void *)bss_list.prev);
	bss = (struct BSS*)((void *)bss_list.next - offsetof(struct BSS, node));
	DBG("bss=%p\n", (void *)bss);
	XASSERT(bss->cookie == BSS_COOKIE, bss->cookie);

	me[5]++;
	bss = bss_new(me);
	list_add(&bss->node, &bss_list);
	me[5]++;
	bss = bss_new(me);
	list_add(&bss->node, &bss_list);
	me[5]++;
	bss = bss_new(me);
	list_add(&bss->node, &bss_list);
	me[5]++;
	bss = bss_new(me);
	list_add(&bss->node, &bss_list);

	struct list_head* tmp;
	char mac[ETH_ALEN+1];
	list_for_each(tmp, &bss_list) {
		bss = list_entry(tmp, struct BSS, node);
		XASSERT(bss->cookie == BSS_COOKIE, bss->cookie);
		mac_addr_n2a(mac, bss->bssid);
		INFO("%s\n", mac);
	}


	return EXIT_SUCCESS;
}

