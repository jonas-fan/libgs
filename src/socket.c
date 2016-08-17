#include "socket.h"
#include "unix_socket.h"

#include <stdlib.h>
#include <string.h>

struct gs_socket_t * gs_socket_create(GS_SOCKET_DOMAIN_TYPE domain)
{
    const struct gs_socket_base_t *base = NULL;

    switch (domain) {
        case GS_SOCKET_DOMAIN_UNIX:
            base = gs_unix_socket_base();
            break;
        default:
            return NULL;
    }

    if (!base) {
        return NULL;
    }

    struct gs_socket_t *gsocket = (struct gs_socket_t *)malloc(sizeof(struct gs_socket_t));

    if (gsocket) {
        memset(gsocket, 0, sizeof(struct gs_socket_t));

        gsocket->domain = domain;
        gsocket->base = base;
    }

    return gsocket;
}

void gs_socket_destroy(struct gs_socket_t *gsocket)
{
    free(gsocket);
}
