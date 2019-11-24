int get_multicast_id(struct nl_sock *sock, const char *family, const char *group);

int print_sta_handler(struct nl_msg *msg, void *arg);

void decode_attr_bss( struct nlattr *attr );

