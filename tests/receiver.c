#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <pthread.h>

#include "../src/gs.h"

#define MAX_MSG_BUFFER_SIZE 512
#define BACKLOG 32
#define MAX_EVENTS 32
#define TIMEOUT 1000

static bool running = false;
static int epoll_fd = -1;

static void print_usage(const char *binary_name)
{
    const char *format = "Usage: %s -b <address>\n"
                         "Options:\n"
                         "  -b    bind and listen the address\n"
                         "        EXAMPLES\n"
                         "        tcp://\033[0;31m127.0.0.1:10000\033[0m\n"
                         "        ipc://\033[0;31m/tmp/uds.ipc\033[0m\n"
                         "\n";

    printf(format, binary_name);
}

static void signal_handler(int number)
{
    running = false;

    printf("Received a signal: 0x%x\n", number);
}

static inline void swap(char *lhs, char *rhs)
{
    const char temp = *lhs;
    *lhs = *rhs;
    *rhs = temp;
}

static void reverse(char *string, unsigned int length)
{
    const unsigned int middle = length >> 1;

    for (unsigned int index = 0; index < middle; ++index) {
        swap(string + index, string + length - index - 1);
    }
}

static void * connection_handler(void *user_data)
{
    pthread_detach(pthread_self());

    struct gs_socket_t *client = (struct gs_socket_t *)user_data;
    char message[MAX_MSG_BUFFER_SIZE];

    const int bytes = gs_recv(client, message, sizeof(message), 0);

    if (bytes > 0) {
        printf("[%p] Receive: '%.*s'\n", (void *)client, bytes, message);

        reverse(message, bytes);

        printf("[%p] Send: '%.*s'\n", (void *)client, bytes, message);

        gs_send(client, message, bytes, 0);
    }

    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.events = EPOLLIN | EPOLLPRI | EPOLLRDHUP;
    event.data.ptr = client;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, gs_raw_fd(client), &event) < 0) {
        printf("epoll_ctl() failed: %s(%d).\n", strerror(errno), errno);
        gs_close(client);
    }

    pthread_exit(NULL);
}

static void do_accept(struct gs_socket_t *gsocket)
{
    struct gs_socket_t *client = gs_accept(gsocket, NULL, 0);

    if (client == NULL) {
        printf("Failed to accept: %s(%d).\n", strerror(errno), errno);
        return;
    }

    printf("New connection (%p)\n", (void *)client);

    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.events = EPOLLIN | EPOLLPRI | EPOLLRDHUP;
    event.data.ptr = client;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, gs_raw_fd(client), &event) < 0) {
        printf("epoll_ctl() failed: %s(%d).\n", strerror(errno), errno);
        gs_close(client);
    }
}

static void do_shutdown(struct gs_socket_t *gsocket)
{
    printf("Close connection (%p)\n", (void *)gsocket);

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, gs_raw_fd(gsocket), NULL);

    gs_close(gsocket);
}

static void do_handle(struct gs_socket_t *gsocket)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, gs_raw_fd(gsocket), NULL);

    pthread_t thread;
    pthread_create(&thread, NULL, connection_handler, gsocket);
}

static void create_server(const char *address, GS_SOCKET_DOMAIN_TYPE type)
{
    struct gs_socket_t *gsocket = gs_socket(type);

    assert(gsocket != NULL);

    if (gs_bind(gsocket, address, BACKLOG) < 0) {
        printf("Failed to bind the address(%s): %s(%d).\n", address, strerror(errno), errno);
        gs_close(gsocket);
        return;
    }

    epoll_fd = epoll_create1(EPOLL_CLOEXEC);

    if (epoll_fd < 0) {
        printf("Failed to create epoll: %s(%d).\n", strerror(errno), errno);
        gs_close(gsocket);
        return;
    }

    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.events = EPOLLIN | EPOLLPRI;
    event.data.ptr = gsocket;

    const int gs_socket_raw_fd = gs_raw_fd(gsocket);

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, gs_socket_raw_fd, &event) < 0) {
        printf("epoll_ctl() failed: %s(%d).\n", strerror(errno), errno);
        close(epoll_fd);
        gs_close(gsocket);
        return;
    }

    struct epoll_event events[MAX_EVENTS];

    printf("[%p] Waiting for incoming connections ...\n", (void *)gsocket);

    while (running) {
        const int num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, TIMEOUT);

        if (num_fds <= 0) {
            continue;
        }

        for (int index = 0; index < num_fds; ++index) {
            if (events[index].data.ptr == gsocket) {
                do_accept(events[index].data.ptr);
            }
            else if (events[index].events & EPOLLRDHUP) {
                do_shutdown(events[index].data.ptr);
            }
            else {
                do_handle(events[index].data.ptr);
            }
        }
    }

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, gs_socket_raw_fd, NULL);

    close(epoll_fd);

    gs_close(gsocket);
}

int main(int argc, char *argv[])
{
    char *address = NULL;

    while (true) {
        const int charactor = getopt(argc, argv, "b:");

        if (charactor == -1) {
            break;
        }

        switch (charactor) {
            case 'b':
                address = strdup(optarg);
                break;
            default:
                free(address);
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!address) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcspn(address, "://") > 3) {
        free(address);
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    GS_SOCKET_DOMAIN_TYPE type[] = {GS_SOCKET_DOMAIN_UNIX, GS_SOCKET_DOMAIN_TCP};
    char *protocols[] = {"ipc://", "tcp://"};

    for (int index = 0; index < GS_SOCKET_DOMAIN_AMOUNT; ++index) {
        if (strncmp(address, protocols[index], strlen(protocols[index])) == 0) {
            running = true;

            signal(SIGINT, signal_handler);
            signal(SIGTERM, signal_handler);

            create_server(strstr(address, "://") + 3, type[index]);

            break;
        }
    }

    printf("Bye ...\n");

    free(address);

    return 0;
}
