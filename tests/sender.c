#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <errno.h>

#include "../src/gs.h"

#define MAX_MSG_BUFFER_SIZE 512

static void print_usage(const char *binary_name)
{
    const char *format = "Usage: %s -c <address> -m <message>\n"
                         "Options:\n"
                         "  -c    connect to the remote address\n"
                         "        EXAMPLES\n"
                         "        tcp://\033[0;31m127.0.0.1:10000\033[0m\n"
                         "        ipc://\033[0;31m/tmp/uds.ipc\033[0m\n"
                         "  -m    transfer message to peer\n"
                         "\n";

    printf(format, binary_name);
}

static void send_message(GS_SOCKET_DOMAIN_TYPE type, const char *address, const char *data, unsigned int data_length)
{
    struct gs_socket_t *gsocket = gs_socket(type);

    assert(gsocket != NULL);

    if (gs_connect(gsocket, address) < 0) {
        printf("Faild to connect: %s(%d).\n", strerror(errno), errno);
        gs_close(gsocket);
        return;
    }

    printf("[%p] Send: '%s'\n", (void *)gsocket, data);

    gs_send(gsocket, data, data_length, 0);

    char message[MAX_MSG_BUFFER_SIZE];

    const int bytes = gs_recv(gsocket, message, sizeof(message), 0);

    if (bytes > 0) {
        printf("[%p] Receive: '%.*s'\n", (void *)gsocket, bytes, message);
    }

    gs_close(gsocket);
}

int main(int argc, char *argv[])
{
    char *address = NULL;
    char *message = NULL;

    while (true) {
        const int charactor = getopt(argc, argv, "b:c:m:");

        if (charactor == -1) {
            break;
        }

        switch (charactor) {
            case 'c':
                address = strdup(optarg);
                break;
            case 'm':
                message = strdup(optarg);
                break;
            default:
                free(message);
                free(address);
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!address || !message) {
        free(address);
        free(message);
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcspn(address, "://") > 3) {
        free(address);
        free(message);
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    GS_SOCKET_DOMAIN_TYPE type[] = {GS_SOCKET_DOMAIN_UNIX, GS_SOCKET_DOMAIN_TCP};
    char *protocols[] = {"ipc://", "tcp://"};

    for (int index = 0; index < GS_SOCKET_DOMAIN_AMOUNT; ++index) {
        if (strncmp(address, protocols[index], strlen(protocols[index])) == 0) {
            send_message(type[index], strstr(address, "://") + 3, message, strlen(message));
            break;
        }
    }

    free(message);
    free(address);

    return 0;
}
