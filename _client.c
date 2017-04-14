/*
 *
 *  MilkChat
 *  Version 1.0
 *
*/

/* Import header file(s) */
#include "_client.h"

/* Global variables */
char listen_host[18];
uint16_t listen_port;

char client_username[MAX_USERNAME_LENGTH];
char client_channel[MAX_CHANNEL_LENGTH];
char client_channel_password[MAX_PASSWORD_LENGTH];

int socket_global;

/* client functions */
int main(int argc, char * argv[]) {
    printf("[#] Welcome to MilkChat - Client!\n\n");

    signal(SIGINT, close_socket);

    if (argc <= 1) {
        print_help();
        return 1;
    }

    if (argc % 2 != 1) {
        print_help();
        return 1;
    }

    if (parse_arguments(argc, argv) != 0) {
        printf("[*] Error. Argument parsing failed!\n");
        return 1;
    }

    if (validate_requirements() != 0) {
        printf("[*] Error. No valid parameters! Enter <program> for arguments.\n");
        return 1;
    }

    printf("[#] Connect to server %s:%d.\n", listen_host, listen_port);

    socket_global = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_global < 0) {
        printf("[*] Error. Cannot opening socket! Aborting.\n");
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(listen_port);

    if (inet_pton(AF_INET, listen_host, &serv_addr.sin_addr) <= 0) {
        printf("[*] Error. inet_pton error occured!\n");
        return 1;
    }

    if (connect(socket_global, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("[*] Error. Error connecting!\n");
        return 1;
    }

    printf("[#] Joining channel %s with username %s.\n", client_channel, client_username);

    CLIENT_INFO client_info;
    strcpy(client_info.username, client_username);
    strcpy(client_info.channel, client_channel);
    strcpy(client_info.channel_password, client_channel_password);

    int x = (int) send(socket_global, &client_info, sizeof(client_info), 0);

    if (x <= 0) {
        printf("[*] Error authenticate to server!\n");
        return 1;
    }

    pthread_t thread1;
    pthread_create(&thread1, NULL, receive_thread, (void *) &socket_global);

    char message[500];

    while (TRUE) {
        bzero(message, 500);
        fgets(message, sizeof(message), stdin);

        int c = message_encoding(message);

        if (c != 0) {
            printf("[*] Encoding message failed!\n");
            break;
        }

        if (strcmp(message, "exit\n") == 0) {
            break;
        }

        if (strlen(message) <= 1) {
            continue;
        }

        int x = send(socket_global, message, strlen(message) + 1, 0);

        if (x <= 0) {
            printf("[*] Error while sending message to server!\n");
            break;
        }
    }

    close_socket();

    return 0;
}

int parse_arguments(int argc, char * argv[]) {
    for (int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "-h") == 0) {
            strcpy(listen_host, argv[i + 1]);
            i++;
        } else if(strcmp(argv[i], "-p") == 0) {
            listen_port = (uint16_t) atoi(argv[i + 1]);
            i++;
        } else if(strcmp(argv[i], "-u") == 0) {
            strcpy(client_username, argv[i + 1]);
            i++;
        } else if(strcmp(argv[i], "-c") == 0) {
            strcpy(client_channel, argv[i + 1]);
            i++;
        } else if(strcmp(argv[i], "-cp") == 0) {
            strcpy(client_channel_password, argv[i + 1]);
            i++;
        }
    }

    return 0;
}

int validate_requirements() {
    if (!listen_port) {
        return 1;
    } else if (strlen(listen_host) < 7) {
        return 1;
    } else if (strlen(listen_host) > 15) {
        return 1;
    } else if (strlen(client_channel_password) > MAX_PASSWORD_LENGTH) {
        return 1;
    } else if (strlen(client_username) < MIN_USERNAME_LENGTH) {
        return 1;
    } else if (strlen(client_username) > MAX_USERNAME_LENGTH) {
        return 1;
    } else if (strlen(client_channel) < MIN_CHANNEL_LENGTH) {
        return 1;
    } else if (strlen(client_channel) > MAX_CHANNEL_LENGTH) {
        return 1;
    }

    return 0;
}

int print_help() {
    printf("Use <program> with following params:\n");
    printf("    -h  <host>              {min: 3, max: 20}\n");
    printf("    -p  <port>              {min: 3, max: 20}\n");
    printf("    -u  <user>              {min: 3, max: 20}\n");
    printf("    -c  <channel>           {min: 3, max: 20}\n");
    printf("    -cp <channel password>  {min: 3, max: 20}\n");

    return 0;
}

int message_encoding(char * message) {
    char encoded_message[500];
    int i, j = 0, k, len = (int) strlen(message);

    for (i = len - 1; i >= 0; i--) {
        if (message[i] != '\n' && message[i] != '\0') {
            encoded_message[j] = message[i];
            j++;
        }
    }

    for (k = 0; k < strlen(encoded_message); k++) {
        if (encoded_message[k] >= 'a' && encoded_message[k] <= 'm') {
            encoded_message[k] += 13;
        } else if (encoded_message[k] >= 'n' && encoded_message[k] <= 'z') {
            encoded_message[k] -= 13;
        } else if (encoded_message[k] >= 'A' && encoded_message[k] <= 'M') {
            encoded_message[k] += 13;
        } else if (encoded_message[k] >= 'N' && encoded_message[k] <= 'Z') {
            encoded_message[k] -= 13;
        }
    }

    encoded_message[j] = '\n';
    encoded_message[j + 1] = '\0';

    bzero(message, 500);
    strcpy(message, encoded_message);

    return 0;
}

void close_socket() {
    printf("[#] Disconnecting from server %s:%d.\n", listen_host, listen_port);
    shutdown(socket_global, 2);

    printf("[#] Thanks for using MilkChat - Client.\n");
    exit(EXIT_SUCCESS);
}

void * receive_thread(void * socket_global) {
    char buffer[500];

    while (TRUE) {
        bzero(buffer, 500);
        ssize_t r = recv(* (int *) socket_global, buffer, (int) sizeof(buffer), 0);

        if (r <= 0) {
            break;
        } else {
            int c = message_encoding(buffer);

            if (c != 0) {
                printf("[*] Message could not be decoded.\n Decoded message: %s", buffer);
            } else {
                printf("[#] %s", buffer);
            }
        }
    }

    return 0;
}
