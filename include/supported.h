#ifndef INCLUDED_supported_h
#define INCLUDED_supported_h

extern void add_isupport(const char *, const char *(*)(const void *), const void *);
extern const void *change_isupport(const char *, const char *(*)(const void *), const void *);
extern void delete_isupport(const char *);
extern void show_isupport(struct Client *);
extern void init_isupport(void);

extern const char *isupport_intptr(const void *);
extern const char *isupport_boolean(const void *);
extern const char *isupport_string(const void *);
extern const char *isupport_stringptr(const void *);

#endif /* INCLUDED_supported_h */
