#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "core.h"
#include "ie.h"

const uint8_t ms_oui[3] = { 0x00, 0x50, 0xf2 };
const uint8_t ieee80211_oui[3] = { 0x00, 0x0f, 0xac };
const uint8_t wfa_oui[3] = { 0x50, 0x6f, 0x9a };

#define POISON 0xee

#define IE_LIST_DEFAULT_MAX 32

static void ie_vendor_free(struct IE* ie)
{
	struct IE_Vendor* vie = (struct IE_Vendor*)ie->specific;

	DBG("%s %p id=%d\n", __func__, (void *)ie, ie->id);

	XASSERT(ie->id==IE_VENDOR, ie->id);

	memset(vie, POISON, sizeof(struct IE_Vendor));
	PTR_FREE(ie->specific);
}

static int ie_vendor_new(struct IE* ie)
{
	struct IE_Vendor* newie;

	DBG("%s id=%d len=%zu oui=0x%02x%02x%02x\n", __func__, 
		ie->id, ie->len, ie->buf[0], ie->buf[1], ie->buf[2]);

	newie = calloc(1, sizeof(struct IE_Vendor));
	if (!newie) { 
		WARN("%s calloc failed\n", __func__);
		return -ENOMEM;
	}

	newie->cookie = IE_SPECIFIC_COOKIE;
	// point to self
	ie->specific = (void *)newie;
	newie->base = ie;
	ie->free = ie_vendor_free;

	memcpy(newie->oui, ie->buf, 3);

	return 0;
}

struct IE* ie_new(uint8_t id, uint8_t len, const uint8_t* buf)
{
	DBG("%s id=%d len=%d\n", __func__, id, len);

	if (id == 0) {
		ERR("%s invalid IE id=%d of len=%d\n", __func__, id, len);
		return NULL;
	}

	struct IE* ie = (struct IE*)calloc(1, sizeof(struct IE));
	if (!ie) {
		return NULL;
	} 

	ie->cookie = IE_COOKIE;
	ie->id = id;
	ie->len = len;
	// keep in mind `buf` doesn't contain the id+len octets
	ie->buf = malloc(len);
	if (!ie->buf) {
		PTR_FREE(ie);
		return NULL;
	}
	memcpy(ie->buf, buf, ie->len);

	if (id==IE_VENDOR) {
		int err = ie_vendor_new(ie);
		if (err) {
			ie_delete(&ie);
			return NULL;
		}
	}

	return ie;
}

void ie_delete(struct IE** pie)
{
	struct IE* ie;

	DBG("%s %p id=%d\n", __func__, (void *)(*pie), (*pie)->id);

	PTR_ASSIGN(ie, *pie);

	XASSERT( ie->cookie == IE_COOKIE, ie->cookie);

	// free my memory
	if (ie->buf) {
		memset(ie->buf, POISON, ie->len);
		PTR_FREE(ie->buf);
	}

	// now let the descendent free its memory
	if (ie->free) {
		ie->free(ie);
	}

	memset(ie, POISON, sizeof(struct IE));
	PTR_FREE(ie);
}

int decode_ie_buf( const uint8_t* ptr, size_t len, struct IE_List* ie_list)
{
	const uint8_t* endptr = ptr + len;
	uint8_t id, ielen;
	int err;

	// assuming caller called ie_list_init()
	XASSERT(ie_list->cookie == IE_LIST_COOKIE, ie_list->cookie);
	XASSERT(ie_list->ieptrlist != NULL, 0);
	XASSERT(ie_list->max != 0, 0);

	while (ptr < endptr) {
		id = *ptr++;
		ielen =  *ptr++;
		DBG("%s id=%u len=%u\n", __func__, id, ielen);

		struct IE* ie = ie_new(id, ielen, ptr);
		if (!ie) {
			ie_list_release(ie_list);
			return -ENOMEM;
		}
		err = ie_list_move_back(ie_list, &ie);
		if (err) {
			ie_delete(&ie);
			ie_list_release(ie_list);
			return err;
		}
		// ie will be NULL at this point

		ptr += ielen;
	}

	return 0;
}

int ie_list_init(struct IE_List* list)
{
	memset(list, 0, sizeof(struct IE_List));
	list->cookie = IE_LIST_COOKIE;
	list->max = IE_LIST_DEFAULT_MAX;
	list->ieptrlist = (struct IE**)calloc(list->max, sizeof(struct IE*));
	if (!list->ieptrlist) {
		return -ENOMEM;
	}
	list->count = 0;

	return 0;
}

void ie_list_release(struct IE_List* list)
{
	XASSERT(list->cookie == IE_LIST_COOKIE, list->cookie);

	for (size_t i=0 ; i<list->count ; i++) {
		XASSERT(list->ieptrlist[i] != NULL, 0);
		ie_delete(&list->ieptrlist[i]);
	}

	memset(list->ieptrlist, POISON, sizeof(struct IE*)*list->max);
	PTR_FREE(list->ieptrlist);
}

int ie_list_move_back(struct IE_List* list, struct IE** pie)
{
	if (list->count+1 > list->max) {
		struct IE** new_ieptrlist;
		INFO("%s resize list=%p from %zu to %zu\n", __func__,
				(void*)list, list->max, list->max*2);
		new_ieptrlist = reallocarray(list->ieptrlist, list->max*2, sizeof(struct IE*));
		if( !new_ieptrlist ) {
			WARN("%s realloc of %zu failed\n", __func__, sizeof(struct IE*)*(list->max*2));
			return -ENOMEM;
		}
		memset(new_ieptrlist+list->max, 0, list->max);
		list->ieptrlist = new_ieptrlist;
		list->max *= 2;
	}

	// dis mine now
	PTR_ASSIGN(list->ieptrlist[list->count++], *pie);

	return 0;
}

