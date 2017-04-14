/*
 *
 *  MilkChat
 *  Version 1.0
 *
*/

/* Import headers */
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

/* Global variables */
#define TRUE 1
#define MIN_USERNAME_LENGTH 3
#define MAX_USERNAME_LENGTH 20
#define MIN_CHANNEL_LENGTH 3
#define MAX_CHANNEL_LENGTH 20
#define MAX_PASSWORD_LENGTH 20

/* Structures */
typedef struct _CLIENT_INFO {
    char username[MAX_USERNAME_LENGTH];
    char channel[MAX_CHANNEL_LENGTH];
    char channel_password[MAX_PASSWORD_LENGTH];
} CLIENT_INFO;

/* Functions */
int parse_arguments(int argc, char * argv[]);
int validate_requirements();
int print_help();
int message_encoding(char * message);
void close_socket();
void * receive_thread(void * socket_global);
