#include "common.h"

#define BUFSZ 500
#define MAX_POKEMON_NAME_LENGTH 10
#define MAX_NUMBER_OF_POKEMON 40

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
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
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    char pokedex[MAX_NUMBER_OF_POKEMON][MAX_POKEMON_NAME_LENGTH];
    // char *pokedex[MAX_POKEMON_NAME_LENGTH];
    int numberOfPokemon = 0;

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        size_t count = recv(csock, buf, BUFSZ - 1, 0);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        char *word = strtok(buf, " ");

        if (strcmp(word, "add") == 0) {
            // Adds a Pokémon, if successfully verified
            while (buf != NULL) {
                word = strtok(NULL, " \n");

                // Buffer ended
                if (word == NULL) {
                    fflush(stdin);
                    break;
                }

                // Command 'kill' stops the server
                if (strcmp(word, "kill") == 0 || strcmp(word, "kill\n") == 0 ||
                    strcmp(word, "kill \n") == 0 ||
                    strcmp(word, "kill ") == 0) {
                    logexit("kill");
                }

                if (numberOfPokemon > 40) {
                    printf("limit exceeded ");
                    break;
                }

                if (strlen(word) > 10) {
                    printf("invalid message ");
                    continue;
                }

                char validCharacters[36] =
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
                if (flag != strlen(word)) {
                    printf("invalid message ");
                    continue;
                }

                // if (alreadyOnPokedex(pokedex, word, numberOfPokemon)) {
                //     printf("%s already exists\n", word);
                //     continue;
                // }

                flag = 0;
                for (int i = 0; i < numberOfPokemon; i++) {
                    if (strcmp(pokedex[i], word) == 0) {
                        flag = 1;
                    }
                }

                if (flag) {
                    printf("%s already exists ", word);
                    continue;
                }

                strcpy(pokedex[numberOfPokemon], word);
                numberOfPokemon++;
                printf("%s added ", word);
            }
            printf("\n");
        } else if (strcmp(word, "remove") == 0) {
            // Removes Pokémon, if successfully verified
            word = strtok(NULL, " \n");

            int flag = 0;
            int pokemonPosition = 0;
            for (int i = 0; i < numberOfPokemon; i++) {
                if (strcmp(pokedex[i], word) == 0) {
                    flag = 1;
                    pokemonPosition = i;
                }
            }

            if (!flag) {
                printf("%s does not exist\n", word);
                break;
            } else {
                for (int i = pokemonPosition; i < numberOfPokemon - 1; i++) {
                    strcpy(pokedex[i], pokedex[i + 1]);
                }
                strcpy(pokedex[numberOfPokemon - 1], "");
                numberOfPokemon--;
                printf("%s removed\n", word);
            }
        } else if (strcmp(word, "exchange") == 0) {
            // Trade two Pokémon, if successfully verified
            // TODO verify the pokemon names
            char *oldPokemon = strtok(NULL, " ");
            char *newPokemon = strtok(NULL, " \n");
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
                printf("%s does not exist\n", oldPokemon);
                break;
            }

            flag = 0;
            for (int i = 0; i < numberOfPokemon; i++) {
                if (strcmp(pokedex[i], newPokemon) == 0) {
                    flag = 1;
                }
            }
            if (flag) {
                printf("%s already exists\n", newPokemon);
                break;
            }

            strcpy(pokedex[pokemonPosition], newPokemon);
            printf("%s exchanged\n", oldPokemon);
        } else if (strcmp(word, "list\n") == 0) {
            for (int i = 0; i < numberOfPokemon; i++) {
                printf("%s ", pokedex[i]);
            }
            printf("\n");
        } else {
            logexit("invalid operation");
        }

        sprintf(buf, "remote endpoint: %.480s\n", caddrstr);
        count = send(csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logexit("send");
        }
        close(csock);
    }

    exit(EXIT_SUCCESS);
}