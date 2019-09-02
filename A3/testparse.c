/*
 * This is a test module for the parser for mysh.
 * This is not part of mysh itself.
 * Compile with parse.c and error.c (only).
 * Type input commands to it and see the parsed data structure.
 * (Arguably this file should not be printing the '$ ' prompt, but it seems
 * easier to use if it does.)
 */

#include <stdio.h>
#include "parse.h"
#include "error.h"

int main()
{
    char buf[1000];
    struct parsed_line *p;
    extern void show(struct parsed_line *p);

    while (printf("$ "), fgets(buf, sizeof buf, stdin)) {
        if ((p = parse(buf))) {
            show(p);
            freeparse(p);
        }
    }

    return(0);
}


void show(struct parsed_line *p)
{
    char **argp;

    if (p->inputfile)
	printf("redirect input from \"%s\"\n ", p->inputfile);

    printf("argv is");
    for (argp = p->argv; *argp; argp++)
	printf(" \"%s\"", *argp);
    printf("\n");

    if (p->outputfile)
	printf("  redirect output to \"%s\"\n", p->outputfile);
}
