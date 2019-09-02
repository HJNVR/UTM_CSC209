struct parsed_line {
    char *inputfile, *outputfile;  /* NULL for no redirection */
    char **argv;
};

extern struct parsed_line *parse(char *s);
extern void freeparse(struct parsed_line *p);
