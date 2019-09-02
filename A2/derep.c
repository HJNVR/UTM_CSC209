#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    int status = 0;
    extern void process(FILE *fp);

    if (argc == 1) {
        process(stdin);
    } else {
        int i;
        FILE *fp;
        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-") == 0) {
                process(stdin);
            } else if ((fp = fopen(argv[i], "r")) == NULL) {
                perror(argv[i]);
                status = 1;
            } else {
                process(fp);
                fclose(fp);
            }
        }
    }
    return(status);
}


void process(FILE *fp)
{
    int lastchar = EOF;  /* something guaranteed not equal to an actual char */
    int count = 0;
    int c;

    while ((c = getc(fp)) != EOF) {
        putchar(c);
        if (c == lastchar) {
            if (++count == 10) {
                for (count = 0; (c = getc(fp)) != EOF && c == lastchar; count++)
                    ;
                if (c != EOF)
                    ungetc(c, fp);
                if (count)
                    printf("[%d more %d bytes]", count, lastchar);
                count = 0;
            }
        } else {
            lastchar = c;
            count = 1;
        }
    }
}
