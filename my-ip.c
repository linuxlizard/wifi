#include <ctype.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>

void hex_dump( const char *label, unsigned char *ptr, int size )
{
    static char hex_ascii[] =
       { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    int i;
    unsigned char line[80];
    unsigned char *ascii, *hex;
    unsigned char *endptr;
    static unsigned long offset=0;

   endptr = ptr + size;
   memset( line, ' ', 80 );
   line[69] = 0;
   while( ptr != endptr ) {
      hex = &line[2];
      ascii = &line[52];
      for( i=0 ; i<16 ; i++ ) {
         if( isprint(*ptr) )
            *ascii++ = *ptr;
         else
            *ascii++ = '.';
         *hex++ = hex_ascii[ *ptr>>4 ];
         *hex++ = hex_ascii[ *ptr&0x0f ];
         *hex++ = ' ';
         ptr++;
         if( ptr == endptr ) {
            /* clean out whatever is left from the last line */
            memset( hex, ' ', (15-i)*3 );
            memset( ascii, ' ', 15-i );
            break;
         }
      }
      printf( "%s 0x%08lx %s\n", label, offset, line );
//      printf( "%d %p %p %s\n", i, ptr, ptr-i, line );
      offset += 16;
   }
}

int main(void)
{
	struct nl_sock* nl_sock = nl_socket_alloc();
	int err = nl_connect(nl_sock, NETLINK_ROUTE);

	struct nl_cache* cache;
	err = rtnl_link_alloc_cache(nl_sock, AF_UNSPEC, &cache);

	struct rtnl_link* link;
	link = rtnl_link_get(cache, 3);

	printf("ifindex=%d name=%s family=%d arptype=%u\n", 
			rtnl_link_get_ifindex(link),
			rtnl_link_get_name(link),
			rtnl_link_get_family(link),
			rtnl_link_get_arptype(link)
		);
	struct nl_addr* addr = rtnl_link_get_addr(link);
	printf("family=%d\n", nl_addr_get_family(addr));
	hex_dump( "addr", nl_addr_get_binary_addr(addr), nl_addr_get_len(addr));

	struct rtnl_addr* rtaddr = rtnl_addr_get(cache, rtnl_link_get_ifindex(link), addr);
	printf("rtaddr=%p\n", rtaddr);

//	struct nl_addr* peer = rtnl_addr_get_peer(rtaddr);
//	if (peer) {
//		hex_dump( "peer", nl_addr_get_binary_addr(peer), nl_addr_get_len(peer));
//	}

	rtnl_link_put(link);
	nl_cache_put(cache);
	nl_socket_free(nl_sock);

	return 0;
}
