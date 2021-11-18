#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define BUFFER_SIZE 500

void logExit(const char *msg);

int addrParse(const char *addrStr, const char *portStr,
              struct sockaddr_storage *storage);

void addrToStr(const struct sockaddr *addr, char *str, size_t strSize);

int serverSockaddrInit(const char *proto, const char *portStr,
                         struct sockaddr_storage *storage);
