#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mysh.h"
#include "parsepath.h"
#include "parse.h"
#include "execute.h"
#include "error.h"

#define MAXPATH 50

int laststatus = 0;  /* set by anything which runs a command */


int main(int argc, char **argv)
{
    char *path, *pathcomponents[MAXPATH];
    int pathsize;
    int c, showprompt = 0, verbose = 0;
    char *executestring = NULL;
    char buf[1000];
    struct parsed_line *p;

    if ((path = getenv("PATH")) == NULL) {
	fprintf(stderr, "mysh: PATH variable not found\n");
	return(1);
    }
    if ((pathsize = parsepath(path, pathcomponents, MAXPATH)) < 0) {
	fprintf(stderr, "mysh: PATH parsing failed (too big?)\n");
	return(1);
    }

    while ((c = getopt(argc, argv, "vic:")) != EOF) {
	switch (c) {
	case 'v':
	    verbose = 1;
	    break;
	case 'i':
	    showprompt = 1;
	    break;
	case 'c':
	    executestring = optarg;
	    break;
	default:
	    laststatus = 1;
	}
    }
    if (laststatus || optind < argc) {
	fprintf(stderr, "usage: %s [-i] [-v] [-c command]\n", argv[0]);
	return(1);
    }
    if (isatty(0))
	showprompt = 1;

    if (executestring) {
	if ((p = parse(executestring)))
	    execute(p, pathcomponents, pathsize);
    } else {
	while (1) {
	    if (showprompt)
		printf("$ ");
	    if (fgets(buf, sizeof buf, stdin) == NULL)
		break;
	    if (verbose)
		fputs(buf, stdout);
	    if ((p = parse(buf))) {
		execute(p, pathcomponents, pathsize);
		freeparse(p);
	    }
	}
    }

    return(laststatus);
}
