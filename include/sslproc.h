#ifndef INCLUDED_sslproc_h
#define INCLUDED_sslproc_h

struct _ssl_ctl;
typedef struct _ssl_ctl ssl_ctl_t;

void init_ssld(void);
int start_ssldaemon(int count, const char *ssl_cert, const char *ssl_private_key, const char *ssl_dh_params);
ssl_ctl_t *start_ssld_accept(rb_fde_t *sslF, rb_fde_t *plainF, int id);
ssl_ctl_t *start_ssld_connect(rb_fde_t *sslF, rb_fde_t *plainF, int id);
void start_zlib_session(void *data);
void send_new_ssl_certs(const char *ssl_cert, const char *ssl_private_key, const char *ssl_dh_params);
void ssld_decrement_clicount(ssl_ctl_t *ctl);
int get_ssld_count(void);

#endif

