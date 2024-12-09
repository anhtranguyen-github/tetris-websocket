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

int join_room(int client_fd, const char *username, const char *room_name, const char *session_id) {
    Message msg = {JOIN_ROOM, "", "", ""};
    snprintf(msg.data, sizeof(msg.data), "%s|%s", session_id, room_name);
    strncpy(msg.username, username, MAX_USERNAME);
    strncpy(msg.room_name, room_name, MAX_ROOM_NAME);

    send(client_fd, &msg, sizeof(Message), 0);

    Message response;
    int bytes_received = recv(client_fd, &response, sizeof(Message), 0);

    if (bytes_received <= 0) {
        printf("Error receiving response from server.\n");
        return 0;
    }

    switch (response.type) {
        case ROOM_JOINED:
            printf("Successfully joined room: %s\n", response.room_name);
            printf("Server message: %s\n", response.data);
            return 1;
        case ROOM_NOT_FOUND:
            printf("Failed to join room: %s\n", response.data);
            break;
        case ROOM_FULL:
            printf("Room is full: %s\n", response.data);
            break;
        case GAME_ALREADY_STARTED:
            printf("Game already started in room: %s\n", response.data);
            break;
        case JOIN_ROOM_FAILURE:
            printf("Failed to join room: %s\n", response.data);
            break;
        default:
            printf("Unexpected response type: %d\n", response.type);
            break;
    }
    return 0;
}

int join_random_room(int client_fd, const char *username, const char *session_id) {
    Message msg = {JOIN_RANDOM, "", "", ""};
    strncpy(msg.username, username, MAX_USERNAME);
    snprintf(msg.data, BUFFER_SIZE, "%s", session_id);

    send(client_fd, &msg, sizeof(Message), 0);

    Message response;
    int bytes_received = recv(client_fd, &response, sizeof(Message), 0);

    if (bytes_received <= 0) {
        printf("Error receiving response from server.\n");
        return 0;
    }

    switch (response.type) {
        case ROOM_JOINED:
            printf("Successfully joined random room: %s\n", response.room_name);
            printf("Server message: %s\n", response.data);
            return 1;
        case ROOM_NOT_FOUND:
            printf("Failed to join a random room: %s\n", response.data);
            break;
        case JOIN_ROOM_FAILURE:
            printf("Unexpected error occurred while joining a random room: %s\n", response.data);
            break;
        default:
            printf("Unexpected response type: %d\n", response.type);
            break;
    }
    return 0;
}