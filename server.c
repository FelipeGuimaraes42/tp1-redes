#include "common.h"

#define MAX_POKEMON_NAME_LENGTH 11
#define MAX_NUMBER_OF_POKEMON 40
#define LETTERS_AND_NUMBERS 36

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

// Command 'kill' stops the server
int isKill(char *word) {
    if (strcmp(word, "kill") == 0 || strcmp(word, "kill\n") == 0 ||
        strcmp(word, "kill \n") == 0 || strcmp(word, "kill ") == 0) {
        return 1;
    }
    return 0;
}

// If the Pokémon name has more than 10 chars, or
// if the Pokémon name has any char other than number and letters
// it must not be added
int isInvalidWord(char *word) {

    if (strlen(word) >= MAX_POKEMON_NAME_LENGTH)
        return 1;

    char validCharacters[LETTERS_AND_NUMBERS] =
        "0123456789abcdefghijklmnopqrstuvwxyz";
    int flag = 0;
    for (int i = 0; i < strlen(word); i++) {
        for (int j = 0; j < strlen(validCharacters); j++) {
            if (word[i] == validCharacters[j]) {
                flag++;
                break;
            }
        }
    }

    // If flag and word size are equal, the word has only valid
    // characters
    if (flag != strlen(word))
        return 1;

    return 0;
}

// int alreadyOnPokedex(char **pokedex, char *pokemon, int numberOfPokemon) {
//     for (int i = 0; i < numberOfPokemon; i++) {
//         if (strcmp(pokedex[i], pokemon) == 0) {
//             return 1;
//         }
//     }
//     return 0;
// }

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != serverSockaddrInit(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int sock;
    sock = socket(storage.ss_family, SOCK_STREAM, 0);
    if (sock == -1) {
        logExit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logExit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(sock, addr, sizeof(storage))) {
        logExit("bind");
    }

    if (0 != listen(sock, 10)) {
        logExit("listen");
    }

    char addrStr[BUFFER_SIZE];
    addrToStr(addr, addrStr, BUFFER_SIZE);
    // printf("bound to %s, waiting connections\n", addrStr);

    char pokedex[MAX_NUMBER_OF_POKEMON][MAX_POKEMON_NAME_LENGTH];
    // char *pokedex[MAX_POKEMON_NAME_LENGTH];
    int numberOfPokemon = 0;

    while (1) {
        struct sockaddr_storage clientStorage;
        struct sockaddr *clientAddr = (struct sockaddr *)(&clientStorage);
        socklen_t clientAddrLength = sizeof(clientStorage);

        int clientSock = accept(sock, clientAddr, &clientAddrLength);
        if (clientSock == -1) {
            logExit("accept");
        }

        char clientAddrStr[BUFFER_SIZE];
        addrToStr(clientAddr, clientAddrStr, BUFFER_SIZE);
        // printf("[log] connection from %s\n", clientAddrStr);

        char buffer[BUFFER_SIZE];

        while (1) {

            memset(buffer, 0, BUFFER_SIZE);
            size_t count = recv(clientSock, buffer, BUFFER_SIZE, 0);
            // printf("[msg] %s, %d bytes: %s\n", clientAddrStr, (int)count,
            // buffer);

            char *word = strtok(buffer, " \n");
            char message[100] = "";
            if (isKill(word)) {
                send(clientSock, word, strlen(word), 0);
                close(clientSock);
                close(sock);
                exit(0);
            } else if (strcmp(word, "add") == 0) {
                // Adds a Pokémon, if successfully verified
                while (buffer != NULL) {
                    word = strtok(NULL, " \n");

                    // Buffer ended
                    if (word == NULL) {
                        fflush(stdin);
                        break;
                    }

                    if (numberOfPokemon >= 40) {
                        strcat(message, "limit exceeded ");
                        continue;
                    }

                    if (isInvalidWord(word)) {
                        strcat(message, "invalid message ");
                        continue;
                    }

                    // if (alreadyOnPokedex(pokedex, word, numberOfPokemon)) {
                    //     printf("%s already exists\n", word);
                    //     continue;
                    // }

                    int flag = 0;
                    for (int i = 0; i < numberOfPokemon; i++) {
                        if (strcmp(pokedex[i], word) == 0) {
                            flag = 1;
                        }
                    }

                    if (flag) {
                        strcat(message, word);
                        strcat(message, " already exists ");
                        continue;
                    }

                    strcpy(pokedex[numberOfPokemon], word);
                    numberOfPokemon++;
                    strcat(message, word);
                    strcat(message, " added ");
                }

            } else if (strcmp(word, "remove") == 0) {
                // Removes Pokémon, if successfully verified
                word = strtok(NULL, " \n");

                if (isInvalidWord(word)) {
                    strcat(message, "invalid message\n");
                    count = send(clientSock, message, strlen(message), 0);
                    if (count != strlen(message)) {
                        logExit("send");
                    }
                    continue;
                }

                int flag = 0;
                int pokemonPosition = 0;

                // Finds or not the Pokémon position
                for (int i = 0; i < numberOfPokemon; i++) {
                    if (strcmp(pokedex[i], word) == 0) {
                        flag = 1;
                        pokemonPosition = i;
                    }
                }

                // If not found, return
                if (!flag) {
                    strcat(message, word);
                    strcat(message, " does not exist");
                } else {
                    // If found, remove the Pokémon and move the other Pokémon
                    // one position back
                    for (int i = pokemonPosition; i < numberOfPokemon - 1;
                         i++) {
                        strcpy(pokedex[i], pokedex[i + 1]);
                    }
                    strcpy(pokedex[numberOfPokemon - 1], "");
                    numberOfPokemon--;
                    strcat(message, word);
                    strcat(message, " removed");
                }

            } else if (strcmp(word, "exchange") == 0) {
                // Trade two Pokémon, if successfully verified
                char *oldPokemon = strtok(NULL, " ");
                char *newPokemon = strtok(NULL, " \n");

                if (isInvalidWord(oldPokemon) || isInvalidWord(newPokemon)) {
                    strcat(message, "invalid message\n");
                    count = send(clientSock, message, strlen(message), 0);
                    if (count != strlen(message)) {
                        logExit("send");
                    }
                    continue;
                }

                int flag = 0;
                int pokemonPosition = 0;

                // Verify if the oldPokemon is registered
                for (int i = 0; i < numberOfPokemon; i++) {
                    if (strcmp(pokedex[i], oldPokemon) == 0) {
                        flag = 1;
                        pokemonPosition = i;
                    }
                }

                if (!flag) {
                    strcat(message, oldPokemon);
                    strcat(message, " does not exist\n");
                    count = send(clientSock, message, strlen(message), 0);
                    if (count != strlen(message)) {
                        logExit("send");
                    }
                    continue;
                }

                flag = 0;
                // Verify if the newPokemon already exists
                for (int i = 0; i < numberOfPokemon; i++) {
                    if (strcmp(pokedex[i], newPokemon) == 0) {
                        flag = 1;
                    }
                }
                if (flag) {
                    strcat(message, newPokemon);
                    strcat(message, " already exists\n");
                    count = send(clientSock, message, strlen(message), 0);
                    if (count != strlen(message)) {
                        logExit("send");
                    }
                    continue;
                }

                strcpy(pokedex[pokemonPosition], newPokemon);

                strcat(message, oldPokemon);
                strcat(message, " exchanged");

            } else if (strcmp(word, "list") == 0) {
                if (numberOfPokemon > 0) {
                    for (int i = 0; i < numberOfPokemon; i++) {
                        strcat(message, pokedex[i]);
                        if (i != numberOfPokemon - 1)
                            strcat(message, " ");
                    }
                } else {
                    strcat(message, "none");
                }
            } else {
                logExit("invalid operation");
            }

            if (message[strlen(message) - 1] == 32) {
                message[strlen(message) - 1] = '\0';
            }
            strcat(message, "\n");
            count = send(clientSock, message, strlen(message), 0);
            if (count != strlen(message)) {
                logExit("send");
            }
        }
    }
    close(sock);
    exit(EXIT_SUCCESS);
}