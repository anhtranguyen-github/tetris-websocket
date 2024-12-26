#ifndef CLIENTUTILS_H
#define CLIENTUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "protocol/network.c"
#include "protocol/protocol.h"
#include "../config/client_config.h"

int send_login_request(int client_fd, const char *username, const char *password, char *session_id);
int create_room(int client_fd, const char *username, const char *room_name, const char *session_id, int time_limit, int brick_limit, int max_player);
int join_room(int client_fd, const char *username, const char *room_name, const char *session_id);
int join_random_room(int client_fd, const char *username, const char *session_id);
int start_game(int client_fd, const char *userName);
int register_user(int client_fd, const char *username, const char *password);

#endif