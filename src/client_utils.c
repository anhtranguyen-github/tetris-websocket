#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "protocol/network.h"
#include "protocol/protocol.h"
#include "../config/client_config.h"

int send_login_request(int client_fd, const char *username, const char *password, char *session_id) {
    Message msg = {LOGIN, "", "", ""};
    strncpy(msg.username, username, MAX_USERNAME);
    strncpy(msg.data, password, BUFFER_SIZE);

    send(client_fd, &msg, sizeof(Message), 0);

    Message response;
    recv(client_fd, &response, sizeof(Message), 0);

    if (response.type == LOGIN_SUCCESS) {
        printf("Login successful! Welcome, %s.\n", username);
        strncpy(session_id, response.data, MAX_SESSION_ID);
        return 1;
    } else if (response.type == LOGIN_FAILURE) {
        printf("Login failed! Please try again.\n");
        return 0;
    } else {
        printf("Unexpected response type: %d\n", response.type);
        return 0;
    }
}

int create_room(int client_fd, const char *username, const char *room_name, const char *session_id, int time_limit, int brick_limit, int max_player) {
    Message msg = {CREATE_ROOM, "", "", ""};
    snprintf(msg.data, sizeof(msg.data), "%s|%d|%d|%d", session_id, time_limit, brick_limit, max_player);
    strncpy(msg.username, username, MAX_USERNAME);
    strncpy(msg.room_name, room_name, MAX_ROOM_NAME);
    send(client_fd, &msg, sizeof(Message), 0);

    return 0;
}

int join_room(int client_fd, const char *username, const char *room_name, const char *session_id) {
    Message msg = {JOIN_ROOM, "", "", ""};
    snprintf(msg.data, sizeof(msg.data), "%s|%s", session_id, room_name);
    strncpy(msg.username, username, MAX_USERNAME);
    strncpy(msg.room_name, room_name, MAX_ROOM_NAME);

    send(client_fd, &msg, sizeof(Message), 0);

    return 0;
}

int join_random_room(int client_fd, const char *username, const char *session_id) {
    Message msg = {JOIN_RANDOM, "", "", ""};
    strncpy(msg.username, username, MAX_USERNAME);
    snprintf(msg.data, BUFFER_SIZE, "%s", session_id);

    send(client_fd, &msg, sizeof(Message), 0);

    return 0;
}

int start_game(int client_fd, const char* userName) {
    Message msg;
    msg.type = START_GAME;
    strncpy(msg.username, userName, MAX_USERNAME);

    // Send the start game request to the server
    if (send(client_fd, &msg, sizeof(msg), 0) < 0) {
        perror("Failed to send start game request");
        return 1;
    }

    return 0;
}

int register_user(int client_fd, const char* username, const char* password) {
    Message msg = {REGISTER, "", "", ""};
    strncpy(msg.username, username, MAX_USERNAME);
    strncpy(msg.data, password, BUFFER_SIZE);

    send(client_fd, &msg, sizeof(Message), 0);

    Message response;
    recv(client_fd, &response, sizeof(Message), 0);

    if (response.type == REGISTER_SUCCESS) {
        printf("Registration successful! Welcome, %s.\n", username);
        return 1;
    }
    else if (response.type == REGISTER_FAILURE) {
        printf("Registration failed: %s\n", response.data);
        return 0;
    }
    else {
        printf("Unexpected response type: %d\n", response.type);
        return 0;
    }
}

int update_score(int client_fd, const char *username, const char *session_id, int score) {
    Message msg = {UPDATE_SCORE, "", "", ""};
    strncpy(msg.username, username, MAX_USERNAME);
    snprintf(msg.data, sizeof(msg.data), "%s|%d", session_id, score);

    if (send(client_fd, &msg, sizeof(Message), 0) < 0) {
        perror("Failed to send update score request");
        return 1;
    }

    printf("Sent update score to server\n");
    return 0;
}

int end_game(int client_fd, const char *username, const char *session_id) {
    Message msg = {END_GAME, "", "", ""};
    strncpy(msg.username, username, MAX_USERNAME);
    
    if (send(client_fd, &msg, sizeof(Message), 0) < 0) {
        perror("Failed to send end game request");
        return 1;
    }

    return 0;
}

int disconnect(int client_fd, const char *username) {
    Message msg = {DISCONNECT, "", "", ""};
    strncpy(msg.username, username, MAX_USERNAME);

    if (send(client_fd, &msg, sizeof(Message), 0) < 0) {
        perror("Failed to send disconnect request\n");
        return 1;
    }

    printf("Successfully send msg to server\n");
    return 0;
} 