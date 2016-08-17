#include "gs.h"
#include "socket.h"

#include <stdlib.h>

struct gs_socket_t * gs_socket(GS_SOCKET_DOMAIN_TYPE domain)
{
    struct gs_socket_t *gsocket = gs_socket_create(domain);

    if (gsocket->base->init(gsocket) < 0) {
        gs_socket_destroy(gsocket);
        return NULL;
    }

    return gsocket;
}

int gs_bind(struct gs_socket_t *gsocket, const char *address, int backlog)
{
    return gsocket->base->bind(gsocket, address, backlog);
}

struct gs_socket_t * gs_accept(struct gs_socket_t *gsocket, char *address, unsigned int length)
{
    struct gs_socket_t *client = gs_socket_create(gsocket->domain);

    if (gsocket->base->accept(gsocket, address, length, client) < 0) {
        gs_socket_destroy(client);
        return NULL;
    }

    return client;
}

int gs_connect(struct gs_socket_t *gsocket, const char *address)
{
    return gsocket->base->connect(gsocket, address);
}

int gs_send(struct gs_socket_t *gsocket, const void *data, unsigned int length, int flags)
{
    return gsocket->base->send(gsocket, data, length, flags);
}

int gs_recv(struct gs_socket_t *gsocket, void *data, unsigned int length, int flags)
{
    return gsocket->base->recv(gsocket, data, length, flags);
}

int gs_raw_fd(struct gs_socket_t *gsocket)
{
    return gsocket->fd;
}

int gs_close(struct gs_socket_t *gsocket)
{
    gsocket->base->close(gsocket);

    gs_socket_destroy(gsocket);

    return 0;
}