/*
 *
 *  MilkChat
 *  Version 1.0
 *
*/

/* Import header file(s) */
#include "_server.h"

/* Global variables */
uint16_t listen_port = 443;

int socket_global, socket_accept, socket_cli_len, client_id = 0;

CHANNEL_INFO * channel_info_ll_head = 0;

/* Server functions */
int delete_channel(CHANNEL_INFO * channel_info_ptr) {
    CHANNEL_INFO * channel_info_previous = NULL;
    CHANNEL_INFO * channel_info_ll_counter = channel_info_ll_head;

    while (TRUE) {
        if (channel_info_ll_counter->channel == channel_info_ptr->channel) {
            if (channel_info_previous) {
                if (channel_info_ll_counter->next == 0) {
                    channel_info_previous->next = 0;
                } else {
                    channel_info_previous->next = channel_info_ll_counter->next;
                }
            } else {
                channel_info_ll_head = 0;
            }

            break;
        } else {
            channel_info_previous = channel_info_ll_counter;
            channel_info_ll_counter = channel_info_ll_counter->next;
        }
    }

    return 0;
}

int delete_channel_user(CLIENT_INFO * client_info, CHANNEL_INFO * channel_info_ptr) {
    CLIENT_INFO *client_info_previous = NULL;
    CLIENT_INFO *client_info_ll_counter = channel_info_ptr->client_info_ll_head;

    if (!client_info_ll_counter) {
        channel_info_ptr->next = 0;
    } else {
        while (TRUE) {
            if (client_info_ll_counter->client_id == client_info->client_id) {
                if (client_info_previous) {
                    if (client_info_ll_counter->next == 0) {
                        client_info_previous->next = 0;
                    } else {
                        client_info_previous->next = client_info_ll_counter->next;
                    }
                } else {
                    if (client_info_ll_counter->next == 0) {
                        channel_info_ptr->next = 0;
                        delete_channel(channel_info_ptr);
                    } else {
                        channel_info_ptr->client_info_ll_head = client_info_ll_counter->next;
                    }
                }

                break;
            } else {
                client_info_previous = client_info_ll_counter;
                client_info_ll_counter = client_info_ll_counter->next;
            }
        }
    }

    return 0;
}

int send_channel_message(CLIENT_INFO * client_info, CHANNEL_INFO * channel_info, char * buffer) {
    CLIENT_INFO * current = channel_info->client_info_ll_head;

    while (TRUE) {
        if (client_info->client_id != current->client_id) {
            send(current->socket_accept, buffer, strlen(buffer) + 1, 0);
        }

        if (current->next == 0) {
            break;
        }

        current = current->next;
    }

    bzero(buffer, 500);
    return 0;
}

void * client_thread(void * client_info_on_connect) {
    char buffer[500];

    CLIENT_INFO *client_info = (CLIENT_INFO *) client_info_on_connect;
    CHANNEL_INFO *channel_info;

    CLIENT_INFO_ACCEPT client_info_accept;
    ssize_t rf = recv(client_info->socket_accept, &client_info_accept, (int) sizeof(client_info_accept), 0);

    if (rf <= 0) {
        return 0;
    } else {
        strcpy(client_info->username, client_info_accept.username);
        strcpy(client_info->channel, client_info_accept.channel);
        strcpy(client_info->channel_password, client_info_accept.channel_password);

        join_channel(client_info, &channel_info);
    }

    if (strcmp(client_info->channel_password, channel_info->channel_password) != 0) {
        char wrong_password[50] = "...grxpbf ajbqghuFanup tabeJ\n";

        send(client_info->socket_accept, wrong_password, strlen(wrong_password) + 1, 0);
        delete_channel_user(client_info, channel_info);
        shutdown(client_info->socket_accept, 2);

        return 0;
    }

    printf("[#] User %s connected!\n", client_info->username);

    while (TRUE) {
        bzero(buffer, 500);
        ssize_t r = recv(client_info->socket_accept, buffer, (int) sizeof(buffer), 0);

        if (r <= 0) {
            printf("[#] User %s disconnected!\n", client_info->username);
            delete_channel_user(client_info, channel_info);
            break;
        }

        int s = send_channel_message(client_info, channel_info, buffer);

        if (s != 0) {
            printf("[*] Failed to send message from user %s in channel %s.\n", client_info->username, client_info->channel);
        }
    }

    return 0;
}

int create_client_info(CLIENT_INFO ** client_info_ptr) {
    CLIENT_INFO *client_info = malloc(sizeof(CLIENT_INFO));
    *client_info_ptr = client_info;
    return 0;
}

int main(int argc, char * argv[]) {
    printf("[#] Welcome to MilkChat — Server!\n\n");

    if (argc < 1) {
        print_help();
        return 1;
    }

    if (parse_arguments() != 0) {
        printf("[*] Error. Argument parsing failed!\n");
        return 1;
    }

    if (validate_requirements() != 0) {
        printf("T*] Error. No valid parameters! Enter <program> for arguments.\n");
        return 1;
    }

    struct sockaddr_in serv_addr, cli_addr;

    socket_global = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_global < 0) {
        printf("[*] Error opening socket! Abortinq.\n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(listen_port);

    if (bind(socket_global, (struct sockaddr *) & serv_addr, sizeof(serv_addr)) < 0) {
        printf("[*] Error while binding socket!\n");
        return 1;
    }

    listen(socket_global, 5);

    while (TRUE) {
        socket_cli_len = sizeof(cli_addr);
        socket_accept = accept(socket_global, (struct sockaddr *) &cli_addr, (socklen_t *) &socket_cli_len);

        if (socket_accept < 0) {
            printf("[*] Error on socket accepting!\n");
            break;
        }

        CLIENT_INFO * client_info_ptr;
        create_client_info(&client_info_ptr);
        client_info_ptr->cli_addr = cli_addr;
        client_info_ptr->socket_accept = socket_accept;
        client_info_ptr->client_id = client_id;

        client_id++;

        pthread_t thread_1;
        pthread_create(&thread_1, NULL, client_thread, (void *) client_info_ptr);
    }
    return 0;
}

int parse_arguments() {
    return 0;
}

int create_channel(CLIENT_INFO * client_info, CHANNEL_INFO ** channel_info_ptr) {
    CHANNEL_INFO * channel_info = malloc(sizeof(CHANNEL_INFO));

    strcpy(channel_info->channel, client_info->channel);
    strcpy(channel_info->channel_owner, client_info->username);
    strcpy(channel_info->channel_password, client_info->channel_password);

    *channel_info_ptr = channel_info;

    return 0;
}

int join_channel(CLIENT_INFO * client_info, CHANNEL_INFO ** channel_info_ptr) {
    CHANNEL_INFO *channel_info;

    if (channel_info_ll_head == 0) {
        create_channel(client_info, &channel_info);
        channel_info_ll_head = channel_info;
    } else {
        CHANNEL_INFO *channel_info_ll_counter = channel_info_ll_head;

        while (TRUE) {
            if (strcmp(channel_info_ll_counter->channel, client_info->channel) == 0) {
                channel_info = channel_info_ll_counter;

                if (!channel_info_ll_head) {
                    strcpy(channel_info->channel_owner, client_info->username);
                    strcpy(channel_info->channel_password, client_info->channel_password);

                    channel_info_ll_head = channel_info;
                }

                break;
            } else if (channel_info_ll_counter->next == 0) {
                create_channel(client_info, &channel_info);
                channel_info_ll_head = channel_info;
                break;
            } else {
                channel_info_ll_counter = channel_info_ll_counter->next;
            }
        }
    }

    if (channel_info->client_info_ll_head == 0) {
        channel_info->client_info_ll_head = client_info;
    } else {
        CLIENT_INFO *client_info_ll_counter = channel_info->client_info_ll_head;

        while (TRUE) {
            if (client_info_ll_counter->next == 0) {
                client_info_ll_counter->next = client_info;
                break;
            } else {
                client_info_ll_counter = client_info_ll_counter->next;
            }
        }
    }

    *channel_info_ptr = channel_info;

    return 0;
}

int validate_requirements() {
    if (!listen_port) {
        return 1;
    }

    return 0;
}

int print_help() {
    printf("Use <program> with following params \n");
    printf(" —p <port> (min: 3, max: 2O}\n");
    return 0;
}
