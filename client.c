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

	printf("connected to %s\n", addrStr);

	char buf[BUFFER_SIZE];
	memset(buf, 0, BUFFER_SIZE);
	printf("mensagem> ");
	fgets(buf, BUFFER_SIZE-1, stdin);
	size_t count = send(sock, buf, strlen(buf)+1, 0);
	if (count != strlen(buf)+1) {
		logExit("send");
	}

	memset(buf, 0, BUFFER_SIZE);
	unsigned total = 0;
	while(1) {
		count = recv(sock, buf + total, BUFFER_SIZE - total, 0);
		if (count == 0) {
			// Connection terminated.
			break;
		}
		total += count;
	}
	close(sock);

	printf("received %u bytes\n", total);
	puts(buf);

	exit(EXIT_SUCCESS);
}