/*
 * Example program to use parsemessage.
 */

#include <stdio.h>
#include <string.h>
#include "parsemessage.h"

int main()
{
    char msg[1000], *p;
    struct ipaddr_port *r;

    printf("Enter the message received.  I'll pretend I'm IP address 1.2.3.4, port 5.\n");
    if (fgets(msg, sizeof msg, stdin) == NULL)
	return(0);
    if ((p = strchr(msg, '\n')))
	*p = '\0';

    setparsemessage(msg);
    while ((r = getparsemessage())
	    && !(r->ipaddr == 0x1020304 && r->port == 5))
	;
    if (r) {
	if (getparsemessage() == NULL)
	    printf("Hey, that message was from me!\n");
	else
	    printf("That message didn't originate with me, but I've already seen it.\n");
    } else {
	printf("I haven't seen that message, so I should now relay it to someone else.\n");
    }
    printf("The history of the message was:\n");
    setparsemessage(msg);
    while ((r = getparsemessage()))
	printf("    IP address %lu.%lu.%lu.%lu, port %d\n",
		(r->ipaddr >> 24) & 255,
		(r->ipaddr >> 16) & 255,
		(r->ipaddr >> 8) & 255,
		r->ipaddr & 255,
		r->port);
    printf("And the message was: %s\n", getmessagecontent());

    return(0);
}
