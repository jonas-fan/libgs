#include "tcp_socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

struct ipv4_address_t {
    char ip[16];
    unsigned int port;
};

static int address_to_ipv4(const char *address, struct ipv4_address_t *ipv4)
{
    if (!address || !ipv4) {
        return -1;
    }

    memset(ipv4, 0, sizeof(struct ipv4_address_t));

    unsigned int ip[4] = {0};
    unsigned int port = 0;
    char postfix[64];

    if (sscanf(address, "%u.%u.%u.%u:%u%s", ip, ip + 1, ip + 2, ip + 3, &port, postfix) != 5) {
        return -1;
    }

    for (unsigned int index = 0; index < 4; ++index) {
        if (ip[index] > 255) {
            return -1;
        }
    }

    if (port > 65536) {
        return -1;
    }

    snprintf(ipv4->ip, sizeof(ipv4->ip), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    ipv4->port = port;

    return 0;
}

static int gs_tcp_socket_init(struct gs_socket_t *gsocket)
{
    gsocket->fd = 0;
    gsocket->address = NULL;

    return 0;
}

static int gs_tcp_socket_bind(struct gs_socket_t *gsocket, const char *address, int backlog)
{
    if (gsocket->fd > 0) {
        return -1;
    }

    struct ipv4_address_t ip_addr;
    memset(&ip_addr, 0, sizeof(struct ipv4_address_t));

    if (address_to_ipv4(address, &ip_addr) != 0) {
        return -1;
    }

    gsocket->fd = socket(AF_INET, SOCK_STREAM, 0);

    if (gsocket->fd <= 0) {
        return -1;
    }

    struct sockaddr_in socket_addr;
    memset(&socket_addr, 0, sizeof(struct sockaddr_in));

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(ip_addr.port);
    socket_addr.sin_addr.s_addr = inet_addr(ip_addr.ip);

    if (bind(gsocket->fd, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) != 0) {
        return -1;
    }

    if (listen(gsocket->fd, backlog) != 0) {
        return -1;
    }

    gsocket->address = strdup(address);

    return 0;
}

static int gs_tcp_socket_accept(struct gs_socket_t *gsocket, char *address, unsigned int length, struct gs_socket_t *client)
{
    struct sockaddr_in client_info;
    memset((void *)&client_info, 0, sizeof(struct sockaddr_in));

    socklen_t sockaddr_len = sizeof(struct sockaddr_in);

    const int client_fd = accept(gsocket->fd, (struct sockaddr *)&client_info, &sockaddr_len);

    if (client_fd <= 0) {
        return -1;
    }

    if (address && length) {
        snprintf(address, length, "%s:%u", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));
    }

    client->fd = client_fd;

    return 0;
}

static int gs_tcp_socket_connect(struct gs_socket_t *gsocket, const char *address)
{
    if (gsocket->fd > 0) {
        return -1;
    }

    struct ipv4_address_t ip_addr;
    memset(&ip_addr, 0, sizeof(struct ipv4_address_t));

    if (address_to_ipv4(address, &ip_addr) < 0) {
        return -1;
    }

    gsocket->fd = socket(AF_INET, SOCK_STREAM, 0);

    if (gsocket->fd <= 0) {
        return -1;
    }

    struct sockaddr_in socket_addr;
    memset(&socket_addr, 0, sizeof(struct sockaddr_in));

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(ip_addr.port);
    socket_addr.sin_addr.s_addr = inet_addr(ip_addr.ip);

    if (connect(gsocket->fd, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) != 0) {
        return -1;
    }

    return 0;
}

static int gs_tcp_socket_send(struct gs_socket_t *gsocket, const void *data, unsigned int length, int flags)
{
    struct iovec iov = {
        .iov_base = (void *)data,
        .iov_len = length
    };

    struct msghdr messagehdr;
    memset((void *)&messagehdr, 0, sizeof(struct msghdr));

    messagehdr.msg_iov = &iov;
    messagehdr.msg_iovlen = 1;
    messagehdr.msg_control = NULL;
    messagehdr.msg_controllen = 0;

    return sendmsg(gsocket->fd, &messagehdr, flags);
}

static int gs_tcp_socket_recv(struct gs_socket_t *gsocket, void *data, unsigned int length, int flags)
{
    struct iovec iov = {
        .iov_base = data,
        .iov_len = length
    };

    struct msghdr messagehdr;
    memset((void *)&messagehdr, 0, sizeof(struct msghdr));

    messagehdr.msg_iov = &iov;
    messagehdr.msg_iovlen = 1;
    messagehdr.msg_control = NULL;
    messagehdr.msg_controllen = 0;

    return recvmsg(gsocket->fd, &messagehdr, flags);
}

static int gs_tcp_socket_close(struct gs_socket_t *gsocket)
{
    close(gsocket->fd);
    free(gsocket->address);

    gsocket->fd = 0;
    gsocket->address = NULL;

    return 0;
}

const struct gs_socket_base_t *gs_tcp_socket_base(void)
{
    static struct gs_socket_base_t base = {
        .init = gs_tcp_socket_init,
        .bind = gs_tcp_socket_bind,
        .accept = gs_tcp_socket_accept,
        .connect = gs_tcp_socket_connect,
        .send = gs_tcp_socket_send,
        .recv = gs_tcp_socket_recv,
        .close = gs_tcp_socket_close
    };

    return &base;
}
