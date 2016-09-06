#ifndef GS_SOCKET_H_
#define GS_SOCKET_H_

#include "domain.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gs_socket_t
{
    const struct gs_socket_base_t *base;
    GS_SOCKET_DOMAIN_TYPE domain;

    int fd;
    char *address;
};

struct gs_socket_base_t
{
    int (*init)(struct gs_socket_t *gsocket);

    int (*close)(struct gs_socket_t *gsocket);

    int (*bind)(struct gs_socket_t *gsocket, const char *address, int backlog);

    int (*accept)(struct gs_socket_t *gsocket, char *address, unsigned int length, struct gs_socket_t *client);

    int (*connect)(struct gs_socket_t *gsocket, const char *address);

    int (*send)(struct gs_socket_t *gsocket, const void *data, unsigned int length, int flags);

    int (*recv)(struct gs_socket_t *gsocket, void *data, unsigned int length, int flags);
};

struct gs_socket_t * gs_socket_create(GS_SOCKET_DOMAIN_TYPE domain);

void gs_socket_destroy(struct gs_socket_t *gsocket);

#ifdef __cplusplus
}
#endif

#endif  /* GS_SOCKET_H_ */