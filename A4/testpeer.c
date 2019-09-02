#include <stdio.h>
#include <stdlib.h>
#include "peer.h"

int main()
{
    int i, n;
    struct peer *p;

    for (i = 0; i < 30; i++) {
	unsigned long ipaddr = random();
	int port = random();
	if (add_peer(ipaddr, port) == NULL)
	    printf("Adding %ld,%d returns error\n", ipaddr, port);
	else
	    printf("Added %ld,%d\n", ipaddr, port);
    }

    n = count_peers();
    printf("%d peers\n", n);

    for (p = top_peer; p; p = p->next)
	printf("a peer is %lu,%d\n", p->ipaddr, p->port);

    for (i = 3; i < n + 10; i += 7) {
	p = nth_peer(i);
	if (p) {
	    printf("%dth peer is %lu,%d\n", i, p->ipaddr, p->port);
	} else {
	    printf("there is no peer #%d\n", i);
	}
    }

    return(0);
}
