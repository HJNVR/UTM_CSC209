/*
 * Test how this program only responds to a full line by comparing:
	(echo -n once; sleep 1; echo upon a time) | cat
 * and
	(echo -n once; sleep 1; echo upon a time) | ./readexample
 * In your assignment, you will use memnewline() or extractnewline() instead
 * of the memchr() call below.  And you'll need to keep track of the "buf" and
 * "bytes_in_buf" values separately for each connected client.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
    char buf[30], *p;
    int len;
    int bytes_in_buf = 0;

    while (bytes_in_buf < sizeof buf - 1 && !memchr(buf, '\n', bytes_in_buf)) {
	if ((len = read(0, buf + bytes_in_buf, sizeof buf - 1 - bytes_in_buf)) < 0) {
	    perror("read");
	    return(1);
	} else if (len == 0) {
	    break;
	} else {
	    bytes_in_buf += len;
	}
    }

    if ((p = memchr(buf, '\n', bytes_in_buf)))
	*p = '\0';
    else
	buf[bytes_in_buf] = '\0';
    printf("%s\n", buf);
    return(0);
}
