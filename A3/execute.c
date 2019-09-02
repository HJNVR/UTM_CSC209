#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "mysh.h"
#include "parse.h"
#include "execute.h"
#include "error.h"


void execute(struct parsed_line *p, char **path, int pathsize)
{
    int i, status;
    char *filepath;
    struct stat statbuf;
    extern char **environ;
    extern char *efilenamecons(char *s, char *t);

    fflush(stdout);
    switch (fork()) {
    case -1:
	perror("fork");
	laststatus = 127;
	break;
    case 0:
	/* child */
	if (p->inputfile) {
	    close(0);
	    if (open(p->inputfile, O_RDONLY, 0) < 0) {
		perror(p->inputfile);
		exit(1);
	    }
	}
	if (p->outputfile) {
	    close(1);
	    if (open(p->outputfile, O_WRONLY|O_CREAT|O_TRUNC, 0666) < 0) {
		perror(p->outputfile);
		exit(1);
	    }
	}
	if (strchr(p->argv[0], '/')) {
	    /* if command contains a '/', don't apply the search path. */
	    filepath = p->argv[0];
	} else {
	    /* find first executable item by that name in the path. */
	    for (i = 0; i < pathsize; i++) {
		filepath = efilenamecons(path[i], p->argv[0]);
		if (stat(filepath, &statbuf) == 0)
		    break;
	    }
	    if (i == pathsize) {
		fprintf(stderr, "%s: Command not found\n", p->argv[0]);
		exit(1);
	    }
	}
	(void)execve(filepath, p->argv, environ);
	perror(filepath);
	exit(1);
    default:
	/* parent */
	wait(&status);
	laststatus = WEXITSTATUS(status);
    }
}


/*
 * Put together a directory name and a file base name, with error checking.
 * Note that the return value is only valid until the next call to
 * efilenamecons().
 */
char *efilenamecons(char *s, char *t)
{
    static char buf[1000];
    if (strlen(s) + 1 + strlen(t) + 1 > sizeof buf)
        fatal("buffer exceeded in efilenamecons()");
    sprintf(buf, "%s/%s", s, t);
    return(buf);
}
