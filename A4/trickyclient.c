#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

static int fd;
enum expectedness { unexpected, intermediateoutput, finaloutput };

int main(int argc, char **argv)
{
    int i, port;
    struct hostent *hp;
    struct sockaddr_in r;
    static char banner[] = "YAK 127.0.0.1 1239\r\n";
    char message[80];
    extern void checkactivity(int sec, int usec, enum expectedness dataexpected);

    if (argc != 3 || (port = atoi(argv[2])) <= 0) {
	fprintf(stderr, "usage: trickyclient hostname portnumber\n");
	return(1);
    }

    if ((hp = gethostbyname(argv[1])) == NULL) {
	fprintf(stderr, "%s: no such host\n", argv[1]);
	exit(1);
    }
    if (hp->h_addr_list[0] == NULL || hp->h_addrtype != AF_INET) {
	fprintf(stderr, "%s: not an internet protocol host name\n", argv[1]);
	exit(1);
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket");
	return(1);
    }
    memset(&r, '\0', sizeof r);
    r.sin_family = AF_INET;
    memcpy(&r.sin_addr, hp->h_addr_list[0], hp->h_length);
    r.sin_port = htons(port);
    if (connect(fd, (struct sockaddr *)&r, sizeof r) < 0) {
	perror("connect");
	return(1);
    }
    snprintf(message, sizeof message, "127.0.0.1,%d;;message", port);

    for (i = 0; i < 3; i++) {
	write(fd, banner + i * 5, 5);
	printf("sent %.5s (but no newline, so the server shouldn't react yet)\n",
		banner + i * 5);
	fflush(stdout);
	sleep(2);
    }
    write(fd, banner + i * 5, 5);
    printf("sent %.3s and newline, so the server should react now\n",
		banner + i * 5);

    checkactivity(2, 0, intermediateoutput);

    printf("Slowing down now...\n");
    printf("Sending a message \"%s\", one character at a time;\nshould only react at the newline\n", message);
    for (i = 0; message[i]; i++) {
	write(fd, message+i, 1);
	putchar(message[i]);
	fflush(stdout);
	checkactivity(0, 200000, unexpected);
	usleep(200000);
    }
    write(fd, "\r\n", 2);
    putchar('\n');

    checkactivity(2, 0, finaloutput);
    printf("No transmissions from the server (this might be fine)\n");
    return(0);
}


void checkactivity(int sec, int usec, enum expectedness dataexpected)
{
    struct timeval tv;
    fd_set fds;
    char buf[1000];
    int len;
    int sawsomething = 0;

    tv.tv_sec = sec;
    tv.tv_usec = usec;
    while (1) {
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	switch (select(fd+1, &fds, NULL, NULL, &tv)) {
	case -1:
	    perror("select");
	    exit(1);
	case 0:
	    if (sawsomething && dataexpected != intermediateoutput)
		exit(0);
	    return;
	}

	if ((len = read(fd, buf, sizeof buf)) < 0) {
	    perror("read");
	    exit(1);
	}
	if (len == 0) {
	    printf("Peer disconnected\n");
	    exit(0);
	}

	sawsomething = 1;
	if (dataexpected)
	    printf("Server says: ");
	else
	    printf("Unexpected data from server: ");
	printf("%.*s\n", len, buf);
    }
}
