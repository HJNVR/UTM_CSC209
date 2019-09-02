#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define STRINGSIZE 10  /* including the terminating \0 */

int verbose = 0;

struct message {
    int key;
    char value[STRINGSIZE];
};

extern int benode(int fromparent, int toparent, int key, char *value);
extern void newnode(int *fromchildp, int *tochildp, pid_t *childpidp, int key, char *value);
extern void parse(char *s, struct message *msgp);
extern void myewrite(int to, void *p, int len);
extern void myeread(int from, void *p, int len);
extern void formatbuf(char *buf, int key, char *value);
extern void mystrncpy(char *s, char *t);


int main(int argc, char **argv)
{
    int c, status = 0;

    while ((c = getopt(argc, argv, "v")) != EOF) {
	switch (c) {
	case 'v':
	    verbose = 1;
	    break;
	default:
	    status = 1;
	}
    }
    if (status || optind + 2 != argc) {
	fprintf(stderr, "usage: %s [-v] key value\n", argv[0]);
	return(1);
    }
    benode(-1, -1, atoi(argv[optind]), argv[optind+1]);
    return(0);
}


int benode(int fromparent, int toparent, int key, char *valuearg)
{
    char buf[100];
    char value[STRINGSIZE];
    int len, tochild[2], fromchild[2];
    pid_t childpid[2];
    struct message msg;

    tochild[0] = tochild[1] = fromchild[0] = fromchild[1] = childpid[0] = childpid[1] = -1;
    mystrncpy(value, valuearg);

    while (1) {
	if (fromparent < 0) {
	    if (fgets(buf, sizeof buf, stdin) == NULL) {
		len = 0;
	    } else {
		parse(buf, &msg);
		len = sizeof(struct message);
	    }
	} else {
	    len = read(fromparent, &msg, sizeof(struct message));
	}
	if (len <= 0) {
	    if (len < 0)
		perror("read");
	    /* and in either case, shut down */
	    if (tochild[0] >= 0)
		close(tochild[0]);
	    if (tochild[1] >= 0)
		close(tochild[1]);
	    while (childpid[0] > 0 || childpid[1] > 0) {
		pid_t pid = wait(NULL);
		if (pid == childpid[0])
		    childpid[0] = -1;
		if (pid == childpid[1])
		    childpid[1] = -1;
	    }
	    if (verbose)
		printf("process with key %d exiting\n", key);
	    return(0);
	}
	/* otherwise, we have a valid and formatted message to send to a
	 * child (or to process ourselves) */
	if (verbose)
	    printf("process with key %d is processing request for key %d\n",
		    key, msg.key);
	if (msg.key == key) {
	    if (msg.value[0])
		mystrncpy(value, msg.value);
	    else
		mystrncpy(msg.value, value);
	} else {
	    int which = (msg.key > key);
	    if (tochild[which] < 0)
		newnode(&fromchild[which], &tochild[which], &childpid[which],
			msg.key, msg.value);
	    myewrite(tochild[which], &msg, sizeof(struct message));
	    myeread(fromchild[which], &msg, sizeof(struct message));
	}
	if (toparent < 0)
	    printf("%d %s\n", msg.key, msg.value);
	else
	    myewrite(toparent, &msg, sizeof(struct message));
    }
}


void newnode(int *fromchildp, int *tochildp, pid_t *childpidp, int key, char *value)
{
    int pipefd[2][2];
    if (pipe(pipefd[0]) || pipe(pipefd[1])) {
	perror("pipe");
	exit(1);
    }
    fflush(stdout);
    switch ((*childpidp = fork())) {
    case -1:
	perror("fork");
	exit(1);
    case 0:
	/* child */
	close(pipefd[0][0]);
	close(pipefd[1][1]);
	exit(benode(pipefd[1][0], pipefd[0][1], key, value));
    default:
	/* parent */
	*fromchildp = pipefd[0][0];
	*tochildp = pipefd[1][1];
	close(pipefd[0][1]);
	close(pipefd[1][0]);
    }
}


void myewrite(int to, void *p, int len)
{
    if (write(to, p, len) != len) {
	perror("write");
	exit(1);
    }
}


void myeread(int from, void *p, int len)  /* for when eof is not expected */
{
    int retval = read(from, p, len);
    if (retval < 0) {
	perror("read");
	exit(1);
    } else if (retval == 0) {
	fprintf(stderr, "unexpected EOF from pipe\n");
	exit(1);
    } else if (retval != len) {
	fprintf(stderr,
"unexpected short read from pipe (expected %d bytes, got %d)\n",
		len, retval);
	exit(1);
    }
}


/* Turn user input into a struct message. */
void parse(char *s, struct message *msgp)
{
    char *p;

    msgp->key = atoi(s);

    if ((p = strchr(s, ' ')) == NULL) {
	msgp->value[0] = '\0';
    } else {
	while (*p && isspace(*p))
	    p++;
	mystrncpy(msgp->value, p);
	if ((p = strchr(msgp->value, '\n')))
	    *p = '\0';
    }
}


void mystrncpy(char *s, char *t)
{
    strncpy(s, t, STRINGSIZE);
    s[STRINGSIZE - 1] = '\0';
}
