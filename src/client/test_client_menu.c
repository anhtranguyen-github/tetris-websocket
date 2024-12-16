#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../protocol/network.c"
#include "../protocol/protocol.h"
#include "../../config/client_config.h"
// Function to display the menu
void print_menu() {
    printf("\nClient Menu:\n");
    printf("1. Login\n");
    printf("2. Create Room\n");
    printf("3. Join Room\n");
    printf("4. Join Room\n");
    printf("5. Disconnect\n");
    printf("6. Exit\n");
    printf("Choose an option: ");
}

// Function to handle login
void login(int client_fd) {
    char username[MAX_USERNAME], password[MAX_USERNAME];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    Message msg = {LOGIN, "", "", ""};
    strncpy(msg.username, username, MAX_USERNAME);
    strncpy(msg.data, password, BUFFER_SIZE);

    send(client_fd, &msg, sizeof(Message), 0);

    Message response;
    recv(client_fd, &response, sizeof(Message), 0);

    if (response.type == LOGIN_SUCCESS) {
        printf("Login successful! Welcome, %s.\n", username);
    } else if (response.type == LOGIN_FAILURE) {
        printf("Login failed! Please try again.\n");
    } else {
        printf("Unexpected response type: %d\n", response.type);
    }
}

// Create room should use generate sessionId from server?
void create_room(int client_fd) {
    char room_name[MAX_ROOM_NAME];
    char session_id[MAX_SESSION_ID];
    char username[MAX_USERNAME];
    int time_limit = DEFAULT_TIME_LIMIT;
    int brick_limit = DEFAULT_BRICK_LIMIT;
    int max_player = DEFAULT_MAX_PLAYER;

    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter room name: ");
    scanf("%s", room_name);
    printf("Enter session ID: ");
    scanf("%s", session_id);
    printf("Enter time limit (-1 for unlimited): ");
    scanf("%d", &time_limit);
    printf("Enter brick limit (-1 for unlimited): ");
    scanf("%d", &brick_limit);
    printf("Enter max player: ");
    scanf("%d", &max_player);

    Message msg = {CREATE_ROOM, "", "", ""};
    // Format the additional data into msg.data as a delimited string
    snprintf(
        msg.data, sizeof(msg.data),
        "%s|%d|%d|%d", 
        session_id, time_limit, brick_limit, max_player
    );

    // Prepare the message to send to the server
    
    strncpy(msg.username, username, MAX_USERNAME);
    strncpy(msg.room_name, room_name, MAX_ROOM_NAME);

    // Send the message to the server
    send(client_fd, &msg, sizeof(Message), 0);

    // Receive the server's response
    Message response;
    int bytes_received = recv(client_fd, &response, sizeof(Message), 0);

    if (bytes_received <= 0) {
        printf("Error receiving response from server.\n");
        return;
    }

    // Handle the server's response
    if (response.type == CREATE_ROOM_SUCCESS) {
        printf("Room created successfully: %s\n", response.data);
    } else if (response.type == CREATE_ROOM_FAILURE) {
        printf("Failed to create room: %s\n", response.data);
    } else {
        printf("Unexpected response type: %d\n", response.type);
    }
}


void join_room(int client_fd) {
    char session_id[MAX_SESSION_ID];
    char username[MAX_USERNAME];
    char room_name[MAX_ROOM_NAME];

    // Prompt user for session ID, username, and room name
    printf("Enter session ID: ");
    scanf("%s", session_id);

    printf("Enter your username: ");
    scanf("%s", username);

    printf("Enter room name to join: ");
    scanf("%s", room_name);

    // Create the JOIN_ROOM message
    Message msg = {0};
    msg.type = JOIN_ROOM;
    strncpy(msg.username, username, MAX_USERNAME);
    strncpy(msg.room_name, room_name, MAX_ROOM_NAME);
    snprintf(msg.data, BUFFER_SIZE, "%s|%s", session_id, room_name);

    // Send JOIN_ROOM message to server
    send(client_fd, &msg, sizeof(Message), 0);

    // Receive response from server
    Message response;
    recv(client_fd, &response, sizeof(Message), 0);

    // Handle the response
    switch (response.type) {
        case ROOM_JOINED:
            printf("Successfully joined room: %s\n", response.room_name);
            printf("Server message: %s\n", response.data);
            break;

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
            printf("Wtf happened but i do not know bruh: %s\n", response.data);
            break;

        default:
            // Print response.type and response.data by default
            printf("Unexpected response type: %d\n", response.type);
            printf("Response Data: %s\n", response.data);
            break;
    }
}


void join_random_room(int client_fd) {
    char session_id[MAX_SESSION_ID];
    char username[MAX_USERNAME];

    // Prompt user for session ID and username
    printf("Enter session ID: ");
    scanf("%s", session_id);

    printf("Enter your username: ");
    scanf("%s", username);

    // Create the JOIN_RANDOM_ROOM message
    Message msg = {0};
    msg.type = JOIN_RANDOM;
    strncpy(msg.username, username, MAX_USERNAME);
    snprintf(msg.data, BUFFER_SIZE, "%s", session_id);  // Only session_id is needed for random room join

    // Send JOIN_RANDOM_ROOM message to server
    send(client_fd, &msg, sizeof(Message), 0);

    // Receive response from server
    Message response;
    recv(client_fd, &response, sizeof(Message), 0);

    // Handle the response
    switch (response.type) {
        case ROOM_JOINED:
            printf("Successfully joined random room: %s\n", response.room_name);
            printf("Server message: %s\n", response.data);
            break;

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
}



// Function to handle disconnection
void disconnect(int client_fd) {
    Message msg = {DISCONNECT, "", "", "Disconnecting"};
    send(client_fd, &msg, sizeof(Message), 0);

    Message response;
    recv(client_fd, &response, sizeof(Message), 0);

    if (response.type == DISCONNECT) {
        printf("Disconnected successfully.\n");
    } else {
        printf("Unexpected response type: %d\n", response.type);
    }
}

// Main client function
void client_function(const char *server_ip) {
    int client_fd = create_client_socket(server_ip);
    if (client_fd < 0) {
        fprintf(stderr, "Client: Failed to connect to server\n");
        exit(EXIT_FAILURE);
    }

    int choice;
    while (1) {
        print_menu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                login(client_fd);
                break;
            case 2:
                create_room(client_fd);
                break;
            case 3:
                join_room(client_fd);
                break;
            case 4:
                join_random_room(client_fd);
                break;
            case 5:
                disconnect(client_fd);
                break;
            case 6:
                printf("Exiting client...\n");
                close(client_fd);
                return;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}

int main() {
    const char *server_ip = "127.0.0.1";  // Change to your server's IP address
    client_function(server_ip);
    return 0;
}
