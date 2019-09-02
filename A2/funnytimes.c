#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

int sflag = 0;
int limit = -1;  /* no limit by default */
int count = 0;


int main(int argc, char **argv)
{
    int c, status = 0;
    extern int process(char *dir);

    while ((c = getopt(argc, argv, "sn:")) != EOF) {
        switch (c) {
        case 's':
            sflag = 1;
            break;
        case 'n':
            limit = atoi(optarg);
            break;
        case '?':
        default:
            status = 1;
            break;
        }
    }

    if (status || optind == argc) {
        fprintf(stderr, "usage: %s [-s] [-n limit] dir ...\n", argv[0]);
        return(1);
    }

    for (; optind < argc; optind++)
        if (process(argv[optind]))
            status = 1;

    return(status);
}


int process(char *dir)
{
#define MAX 256
    char subpath[strlen(dir) + 1 + MAX + 1];
    char *p;
    DIR *dp;
    struct dirent *r;
    struct stat statbuf;
    int status = 0;

    strcpy(subpath, dir);
    strcat(subpath, "/");
    p = strchr(subpath, '\0');
    if ((dp = opendir(dir)) == NULL) {
        perror(dir);
        return(1);
    }

    while ((r = readdir(dp))) {
        if (strcmp(r->d_name, ".") == 0 || strcmp(r->d_name, "..") == 0) {
            /* skip */
        } else if (strlen(r->d_name) > MAX) {
            /*
             * Directory entries are always less than MAX bytes on the
             * computer I'm writing this on, but check so as to handle the
             * general case.
             */
            fprintf(stderr, "%s/%s: pathname too long\n", dir, r->d_name);
            status = 1;
        } else {
            strcpy(p, r->d_name);
            if (lstat(subpath, &statbuf)) {
                perror(subpath);
                status = 1;
            } else {
                if ((sflag && statbuf.st_mtime < statbuf.st_ctime)
                        || (!sflag && statbuf.st_mtime != statbuf.st_ctime)) {
                    if (limit >= 0 && limit < ++count)
                        exit(2);
                    printf("%s\n", subpath);
                }
                if (S_ISDIR(statbuf.st_mode))
                    status |= process(subpath);
            }
        }
    }

    closedir(dp);
    return(status);
}

