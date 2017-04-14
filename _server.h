/*
 *
 *  MilkChat
 *  Version 1.0
 *
*/

/* Import headers */
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/* Global variables */
#define TRUE 1
#define MAX_USERNAME_LENGTH 20
#define MAX_CHANNEL_LENGTH 20
#define MAX_PASSWORD_LENGTH 20

typedef struct _CLIENT_INFO CLIENT_INFO;
typedef struct _CHANNEL_INFO CHANNEL_INFO;

/* Structures */
typedef struct _CLIENT_INFO {
    CLIENT_INFO * next;
    char username[MAX_USERNAME_LENGTH];
    char channel[MAX_CHANNEL_LENGTH];
    char channel_password[MAX_PASSWORD_LENGTH];
    struct sockaddr_in cli_addr;
    int socket_accept;
    int client_id;
} CLIENT_INFO;

typedef struct _CLIENT_INFO_ACCEPT {
    char username[MAX_USERNAME_LENGTH];
    char channel[MAX_CHANNEL_LENGTH];
    char channel_password[MAX_PASSWORD_LENGTH];
} CLIENT_INFO_ACCEPT;

typedef struct _CHANNEL_INFO {
    CHANNEL_INFO * next;
    char channel[MAX_CHANNEL_LENGTH];
    char channel_password[MAX_CHANNEL_LENGTH];
    char channel_owner[MAX_CHANNEL_LENGTH];
    CLIENT_INFO * client_info_ll_head;
} CHANNEL_INFO;

/* Functions */
int parse_arguments();
int validate_requirements();
int print_help();
int join_channel(CLIENT_INFO * client_info, CHANNEL_INFO ** channel_info_ptr);
int create_channel(CLIENT_INFO * client_info, CHANNEL_INFO ** channel_info_ptr);
int create_client_info(CLIENT_INFO ** client_info_ptr);
int send_channel_message(CLIENT_INFO * client_info, CHANNEL_INFO * channel_info, char * buffer);
void * client_thread(void * client_info_on_connect);
int delete_channel(CHANNEL_INFO * channel_info_ptr);
int delete_channel_user(CLIENT_INFO * client_info, CHANNEL_INFO * channel_info_ptr);
