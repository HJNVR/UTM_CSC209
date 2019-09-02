#include <string.h>
#include <stdlib.h>

int parsepath(char *path, char **result, int size)
{
    char *p;
    int n = 0;
    while (n < size) {
        if ((p = strchr(path, ':'))) {
            if (p == path) {
                result[n++] = ".";
            } else {
                int size = p - path;
                if ((result[n] = malloc(size + 1)) == NULL)
                    return(-1);
                strncpy(result[n], path, size);
                result[n++][size] = '\0';
            }
            path = p + 1;
        } else {
            if (path[0] == '\0')
                result[n++] = ".";
            else
                result[n++] = path;
            return(n);
        }
    }
    /*
     * since the loop exits when we have no more path items, getting here
     * means that we're out of space in the result array
     */
    return(-1);
}
