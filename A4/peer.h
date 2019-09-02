#define MAXMESSAGE 1000

struct peer {
    unsigned long ipaddr;
    int port;
    int fd;
    int saw_banner;
    char buf[MAXMESSAGE+1];
    int bytes_in_buf;  /* how many data bytes in buf (after nextpos) */
    char *nextpos;  /* if non-NULL, move this down to buf[0] before reading */
    struct peer *next;
};  
extern struct peer *add_peer(unsigned long ipaddr, int port);
extern void delete_peer(struct peer *p);
extern struct peer *find_peer(unsigned long ipaddr, int port);
extern int count_peers();
extern struct peer *nth_peer(int n), *random_peer();
extern struct peer *top_peer;
