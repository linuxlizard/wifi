#ifndef IW_SCAN_H
#define IW_SCAN_H

#include "bytebuf.h"

int get_multicast_id(struct nl_sock* sock, const char* family, const char* group);

int print_sta_handler(struct nl_msg* msg, void *arg);

void decode_attr_bss( struct nlattr* attr );
int decode_attr_scan_frequencies( struct nlattr *attr);
int decode_attr_scan_ssids( struct nlattr* attr, struct bytebuf_array* ssid_list  ) ;

#endif
