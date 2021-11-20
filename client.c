#include "common.h"

void usage(int argc, char **argv) {
    printf("usage: %s <server IP> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != addrParse(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int sock;
    sock = socket(storage.ss_family, SOCK_STREAM, 0);
    if (sock == -1) {
        logExit("socket");
    }
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != connect(sock, addr, sizeof(storage))) {
        logExit("connect");
    }

    char addrStr[BUFFER_SIZE];
    addrToStr(addr, addrStr, BUFFER_SIZE);

    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE - 1, stdin);
        size_t count = send(sock, buffer, strlen(buffer), 0);
        if (count != strlen(buffer)) {
            logExit("send");
        }

        memset(buffer, 0, BUFFER_SIZE);

        recv(sock, buffer, BUFFER_SIZE, 0);

        if (strcmp(buffer, "kill") == 0) {
            break;
        }
        puts(buffer);
    }

    close(sock);
    exit(EXIT_SUCCESS);
}