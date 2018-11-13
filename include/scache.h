#ifndef INCLUDED_scache_h
#define INCLUDED_scache_h

struct Client;
struct scache_entry;

extern void clear_scache_hash_table(void);
extern struct scache_entry *scache_connect(const char *name, const char *info, int hidden);
extern void scache_split(struct scache_entry *ptr);
extern const char *scache_get_name(struct scache_entry *ptr);
extern void scache_send_flattened_links(struct Client *source_p);
extern void scache_send_missing(struct Client *source_p);
extern void count_scache(size_t *, size_t *);

#endif
