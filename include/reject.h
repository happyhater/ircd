#ifndef INCLUDED_reject_h
#define INCLUDED_reject_h

/* amount of time to delay a rejected clients exit */
#define DELAYED_EXIT_TIME	10

void init_reject(void);
int check_reject(rb_fde_t *F, struct sockaddr *addr);
void add_reject(struct Client *, const char *mask1, const char *mask2);
int is_reject_ip(struct sockaddr *addr);
void flush_reject(void);
int remove_reject_ip(const char *ip);
int remove_reject_mask(const char *mask1, const char *mask2);
unsigned long delay_exit_length(void);

int throttle_add(struct sockaddr *addr);
int is_throttle_ip(struct sockaddr *addr);
unsigned long throttle_size(void);
void flush_throttle(void);


#endif

