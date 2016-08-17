#ifndef GS_H_
#define GS_H_

#include "domain.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gs_socket_t * gs_socket(GS_SOCKET_DOMAIN_TYPE domain);

int gs_bind(struct gs_socket_t *gsocket, const char *address, int backlog);

struct gs_socket_t * gs_accept(struct gs_socket_t *gsocket, char *address, unsigned int length);

int gs_connect(struct gs_socket_t *gsocket, const char *address);

int gs_send(struct gs_socket_t *gsocket, const void *data, unsigned int length, int flags);

int gs_recv(struct gs_socket_t *gsocket, void *data, unsigned int length, int flags);

int gs_raw_fd(struct gs_socket_t *gsocket);

int gs_close(struct gs_socket_t *gsocket);

#ifdef __cplusplus
}
#endif

#endif  /* GS_H_ */