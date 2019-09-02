#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <netdb.h>
#include "peer.h"
#include "parsemessage.h"
#include "util.h"

#define DEFPORT 3000  /* (default both for connecting and for listening) */

unsigned long ipaddr = 0;  /* 0 means not known yet */
int myport = DEFPORT;
int relaymax = 10;
int verbose = 0;

static int listenfd;


int main(int argc, char **argv)
{
    int c;
    struct peer *p, *nextp;
    fd_set fdlist;
    int maxfd;
    extern void doconnect(unsigned long ipaddr, int port), bindandlisten();
    extern void newconnection(), dostdin(), read_and_process(struct peer *p);
    extern unsigned long hostlookup(char *host);

    while ((c = getopt(argc, argv, "p:c:v")) != EOF) {
	switch (c) {
	case 'p':
	    if ((myport = atoi(optarg)) == 0) {
		fprintf(stderr, "%s: port number must be a positive integer\n",
			argv[0]);
		return(1);
	    }
	    break;
	case 'c':
	    relaymax = atoi(optarg);
	    break;
	case 'v':
	    verbose = 1;
	    break;
	default:
	    fprintf(stderr, "usage: %s [-p port] [-c relaymax] [-v] [host [port]]\n", argv[0]);
	    return(1);
	}
    }

    if (optind < argc) {
	optind++;
	doconnect(hostlookup(argv[optind - 1]),
		(optind < argc) ? atoi(argv[optind]) : DEFPORT);
    }

    bindandlisten();  /* aborts on error */

    /* the only way the server exits is by being killed */
    for (;;) {
	FD_ZERO(&fdlist);
	FD_SET(0, &fdlist);
	FD_SET(listenfd, &fdlist);
	maxfd = listenfd;
	for (p = top_peer; p; p = p->next) {
	    FD_SET(p->fd, &fdlist);
	    if (p->fd > maxfd)
		maxfd = p->fd;
	}
	if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
	    perror("select");
	} else {
	    if (FD_ISSET(listenfd, &fdlist))
		newconnection();
	    if (FD_ISSET(0, &fdlist))
		dostdin();
	    for (p = top_peer; p; p = nextp) {
		nextp = p->next;  /* in case we remove this peer because of error */
		if (FD_ISSET(p->fd, &fdlist))
		    read_and_process(p);
	    }
	}
    }
}


void doconnect(unsigned long ipaddr, int port)
{
    struct sockaddr_in r;
    struct peer *p;
    char msg[100];

    memset(&r, '\0', sizeof r);
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = htonl(ipaddr);
    r.sin_port = htons(port);
    p = add_peer(ipaddr, port);
    if (verbose)
	printf("Connecting to %s, port %d\n", inet_ntoa(r.sin_addr), port);
    if ((p->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket");
	exit(1);
    }
    if (connect(p->fd, (struct sockaddr *)&r, sizeof r) < 0) {
	perror("connect");
	exit(1);
    }

    p->bytes_in_buf = 0;
    p->nextpos = NULL;
    p->saw_banner = 0;
    sprintf(msg, "YAK %s %d\r\n", inet_ntoa(r.sin_addr), myport);
    write(p->fd, msg, strlen(msg));
}


void bindandlisten()  /* bind and listen, abort on error */
{
    struct sockaddr_in r;

    (void)signal(SIGPIPE, SIG_IGN);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket");
	exit(1);
    }

    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(myport);

    if (bind(listenfd, (struct sockaddr *)&r, sizeof r)) {
	perror("bind");
	exit(1);
    }

    if (listen(listenfd, 5)) {
	perror("listen");
	exit(1);
    }
}


unsigned long hostlookup(char *host)
{
    struct hostent *hp;
    struct in_addr a;

    if ((hp = gethostbyname(host)) == NULL) {
        fprintf(stderr, "%s: no such host\n", host);
        exit(1);
    }
    if (hp->h_addr_list[0] == NULL || hp->h_addrtype != AF_INET) {
        fprintf(stderr, "%s: not an internet protocol host name\n", host);
        exit(1);
    }
    memcpy(&a, hp->h_addr_list[0], hp->h_length);
    return(ntohl(a.s_addr));
}


void newconnection()  /* accept connection, add peer */
{
    int fd;
    struct sockaddr_in r;
    socklen_t len = sizeof r;

    if ((fd = accept(listenfd, (struct sockaddr *)&r, &len)) < 0) {
	perror("accept");
    } else {
	char buf[140];
	struct peer *p = add_peer(ntohl(r.sin_addr.s_addr), 0);
	p->fd = fd;
	p->bytes_in_buf = 0;
	p->nextpos = NULL;
	p->saw_banner = 0;
	sprintf(buf, "YAK %s\r\n", inet_ntoa(r.sin_addr));
	printf("new connection from %s, fd %d\n", inet_ntoa(r.sin_addr), fd);
	fflush(stdout);
	write(fd, buf, strlen(buf));
    }
}


/* select() said activity; check it out */
void read_and_process(struct peer *p)
{
    char *msg;
    struct ipaddr_port *r, *prev;
    int left;
    extern char *myreadline(struct peer *p);
    extern int analyze_banner(char *s, struct peer *p);

    if ((msg = myreadline(p)) == NULL)
	return;
    if (!p->saw_banner) {
	p->saw_banner = 1;
	if (strncmp(msg, "YAK ", 4) || analyze_banner(msg + 4, p)) {
	    printf("Bad initial message from %d; dropping this peer\n", p->fd);
	    close(p->fd);
	    delete_peer(p);
	}
	return;
    }
    if (verbose)
	printf("Received message to evaluate: %s\n", msg);
    setparsemessage(msg);
    r = NULL;
    left = relaymax;
    while (prev = r, (r = getparsemessage()) && left-- > 0)
	;
    if (r || (prev && prev->ipaddr == ipaddr && prev->port == myport)) {
	if (r)
	    printf("Here's a message which we're not going to relay because the relaymax count has been reached:\n");
	else
	    printf("Here's a message from you!\n");
	printf("The history of the message was:\n");
	setparsemessage(msg);
	while ((r = getparsemessage())) {
	    if (r->ipaddr == ipaddr && r->port == myport)
		printf("    you\n");
	    else
		printf("    IP address %s, port %d\n",
			format_ipaddr(r->ipaddr), r->port);
	}
	printf("And the message was: %s\n", getmessagecontent());
    } else {
	struct peer *p = random_peer();
	if (p) {
	    char buf[100];
	    sprintf(buf, "%s,%d;", format_ipaddr(ipaddr), myport);
	    write(p->fd, buf, strlen(buf));
	    write(p->fd, msg, strlen(msg));
	    write(p->fd, "\r\n", 2);
	    if (verbose)
		printf("relaying to %s, port %d\n", format_ipaddr(p->ipaddr), p->port);
	} else {
	    printf("No one to relay to!\n");
	}
    }

    /* Any new peers? */
    setparsemessage(msg);
    while ((r = getparsemessage())) {
	if (!(r->ipaddr == ipaddr && r->port == myport) && !find_peer(r->ipaddr, r->port)) {
	    printf("Hey, a message mentions %s %d, which I haven't heard of!\n", format_ipaddr(r->ipaddr), r->port);
	    doconnect(r->ipaddr, r->port);
	}
    }
}


void dostdin()
{
    char msg[MAXMESSAGE], *mp;
    struct peer *p;

    sprintf(msg, "%s,%d;;", format_ipaddr(ipaddr), myport);
    mp = strchr(msg, '\0');
    /* leaving space for the \0 and also space to change \n -> \r\n */
    if (fgets(mp, sizeof msg - (mp - msg) - 2, stdin) == NULL)
	exit(0);
    if (mp[0] == '\n') {
	printf("%d peers:\n", count_peers());
	for (p = top_peer; p; p = p->next)
	    printf("peer on fd %d is on port %d of %s\n",
		    p->fd, p->port,
		    format_ipaddr(p->ipaddr));
	printf("end of peer list\n");
    } else {
	if ((mp = strchr(msg, '\n')))
	    strcpy(mp, "\r\n");
	p = random_peer();
	if (p) {
	    if (verbose)
		printf("Sending to %s %d: %s\n", format_ipaddr(p->ipaddr),
			p->port, msg);
	    write(p->fd, msg, strlen(msg));
	} else {
	    printf("No one to send to!\n");
	}
    }
}


char *myreadline(struct peer *p)
{
    int nbytes;

    /* move the leftover data to the beginning of buf */
    if (p->bytes_in_buf && p->nextpos)
	memmove(p->buf, p->nextpos, p->bytes_in_buf);

    /* If we've already got another whole line, return it without a read() */
    if ((p->nextpos = extractline(p->buf, p->bytes_in_buf))) {
	p->bytes_in_buf -= (p->nextpos - p->buf);
	return(p->buf);
    }

    /*
     * Ok, try a read().  Note that we _never_ fill the buffer, so that there's
     * always room for a \0.
     */
    nbytes = read(p->fd, p->buf + p->bytes_in_buf, sizeof p->buf - p->bytes_in_buf - 1);
    if (nbytes <= 0) {
	if (nbytes < 0)
	    perror("read()");
	printf("Disconnecting fd %d, ipaddr %s, port %d\n", p->fd,
		format_ipaddr(p->ipaddr), p->port);
	fflush(stdout);
	close(p->fd);
	delete_peer(p);
    } else {

	p->bytes_in_buf += nbytes;

	/* So, _now_ do we have a whole line? */
	if ((p->nextpos = extractline(p->buf, p->bytes_in_buf))) {
	    p->bytes_in_buf -= (p->nextpos - p->buf);
	    return(p->buf);
	}

	/*
	 * Don't do another read(), to avoid the possibility of blocking.
	 * However, if we've hit the maximum message size, we should call
	 * it all a line.
	 */
	if (p->bytes_in_buf >= MAXMESSAGE) {
	    p->buf[p->bytes_in_buf] = '\0';
	    p->bytes_in_buf = 0;
	    p->nextpos = NULL;
	    return(p->buf);
	}

    }

    /* If we got to here, we don't have a full input line yet. */
    return(NULL);
}


int analyze_banner(char *s, struct peer *p)
{
    unsigned long a, b, c, d, newipaddr;
    int numfields;
    int newport;

    numfields = sscanf(s, "%lu.%lu.%lu.%lu %d", &a, &b, &c, &d, &newport);
    if (numfields < 4) {
	fprintf(stderr, "'%s' does not begin with an IP address\n", s);
	return(-1);
    }

    newipaddr = (a << 24) | (b << 16) | (c << 8) | d;
    if (ipaddr == 0) {
	ipaddr = newipaddr;
	printf("I've learned that my IP address is %s\n",
		format_ipaddr(ipaddr));
    } else if (ipaddr != newipaddr) {
	fprintf(stderr,
"fatal error: I thought my IP address was %s, but newcomer says it's %s\n",
		format_ipaddr(ipaddr), s);
	exit(1);
    }

    if (numfields > 4) {
	if (p->port == 0) {
	    struct peer *q = find_peer(p->ipaddr, newport);
	    if (q == NULL) {
		p->port = newport;
		printf(
"I've learned that the peer on fd %d's port number is %d\n",
			p->fd, p->port);
	    } else {
		printf(
"fd %d's port number is %d, so it's a duplicate of fd %d, so I'm dropping it.\n",
			p->fd, newport, q->fd);
		close(p->fd);
		delete_peer(p);
	    }
	} else if (p->port != newport) {
	    printf(
"I'm a bit concerned because I thought the peer on fd %d's port number was %d, but it says it's %d\n",
		    p->fd, p->port, newport);
	}
    }

    return(0);
}
