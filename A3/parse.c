#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "parse.h"
#include "error.h"


#define MAXARGV 1000

enum token {
    word, directin, directout,
    eol
};
static enum token gettoken(char **s, char **argp);


struct parsed_line *parse(char *s)
{
    struct parsed_line *retval;  /* remains freeparse()able at all times */
    char *argv[MAXARGV];
    enum token tok;
    int argc = 0;

    retval = emalloc(sizeof(struct parsed_line));
    retval->inputfile = retval->outputfile = NULL;
    retval->argv = NULL;

    while ((tok = gettoken(&s, &argv[argc])) < eol) {
	switch ((int)tok) {  /* cast prevents stupid warning message about
			      * not handling all enum token values */
	case word:
	    argc++;  /* it's already in argv[argc];
		      * increment to represent a save */
	    if (argc == MAXARGV - 1)
		fatal("argv limit exceeded");
	    break;
	case directin:
	    if (retval->inputfile) {
		fprintf(stderr,
			"syntax error: multiple input redirections\n");
		freeparse(retval);
		return(NULL);
	    }
	    if (gettoken(&s, &retval->inputfile) != word) {
		fprintf(stderr, "syntax error in input redirection\n");
		freeparse(retval);
		return(NULL);
	    }
	    break;
	case directout:
	    if (retval->outputfile) {
		fprintf(stderr,
			"syntax error: multiple output redirections\n");
		freeparse(retval);
		return(NULL);
	    }
	    if (gettoken(&s, &retval->outputfile) != word) {
		fprintf(stderr, "syntax error in output redirection\n");
		freeparse(retval);
		return(NULL);
	    }
	    break;
	}
    }

    /* cons up just-parsed argv */
    if (argc)
	retval->argv = eargvsave(argv, argc);

    return(retval);
}


/* (*s) is advanced as we scan; *argp is set iff retval == word */
static enum token gettoken(char **s, char **argp)
{
    char *p;

    while (**s && isascii(**s) && isspace(**s))
        (*s)++;
    switch (**s) {
    case '\0':
        return(eol);
    case '<':
        (*s)++;
        return(directin);
    case '>':
        (*s)++;
        return(directout);
    default:
	/* it's a word */
	/* find the beginning and end of the word */
	p = *s;
	while (**s && isascii(**s) && !isspace(**s) && !strchr("<>", **s))
	    (*s)++;
	*argp = estrsavelen(p, *s - p);
	return(word);
    }
}


void freeparse(struct parsed_line *p)
{
    if (p) {
	if (p->inputfile)
	    free(p->inputfile);
	if (p->outputfile)
	    free(p->outputfile);
	if (p->argv) {
	    char **ap;
	    for (ap = p->argv; *ap; ap++)
		free(*ap);
	    free(p->argv);
	}
	free(p);
    }
}
