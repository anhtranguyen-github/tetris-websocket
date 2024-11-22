#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../protocol/network.c"
#include "../protocol/protocol.h"

// Function to display the menu
void print_menu() {
    printf("\nClient Menu:\n");
    printf("1. Login\n");
    printf("2. Create Room\n");
    printf("3. Join Room\n");
    printf("4. Disconnect\n");
    printf("5. Exit\n");
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

// Function to handle room creation
void create_room(int client_fd) {
    char room_name[MAX_ROOM_NAME];
    char session_id[MAX_SESSION_ID];  // Store session ID here

    // Get the room name and session ID from the user
    printf("Enter room name: ");
    scanf("%s", room_name);  // Get room name input from the user
    printf("Enter session ID: ");
    scanf("%s", session_id);  // Get session ID input from the user (this should be passed by the client)

    // Prepare the message to send to the server
    Message msg = {CREATE_ROOM, "", "", ""};
    strncpy(msg.room_name, room_name, MAX_ROOM_NAME);  // Set room name in the message
    strncpy(msg.data, session_id, MAX_SESSION_ID);     // Set session ID (as msg.data)

    // Send the message to the server
    send(client_fd, &msg, sizeof(Message), 0);

    // Receive the server's response
    Message response;
    int bytes_received = recv(client_fd, &response, sizeof(Message), 0);

    if (bytes_received <= 0) {
        printf("Error receiving response from server.\n");
        return;
    }

    // Handle the server's response based on the response type
    if (response.type == JOIN_ROOM_SUCCESS) {
        printf("Room created successfully: %s\n", response.data);  // Success response
    } else if (response.type == JOIN_ROOM_FAILURE) {
        printf("Failed to create room: %s\n", response.data);  // Failure response
    } else {
        printf("Unexpected response type: %d\n", response.type);  // Unexpected response
    }
}

// Function to handle room joining
void join_room(int client_fd) {
    char room_name[MAX_ROOM_NAME];
    printf("Enter room name to join: ");
    scanf("%s", room_name);

    Message msg = {JOIN_ROOM, "", "", ""};
    strncpy(msg.room_name, room_name, MAX_ROOM_NAME);

    send(client_fd, &msg, sizeof(Message), 0);

    Message response;
    recv(client_fd, &response, sizeof(Message), 0);

    if (response.type == ROOM_JOINED) {
        printf("Successfully joined room: %s\n", room_name);
    } else if (response.type == ROOM_NOT_FOUND) {
        printf("Failed to join room: %s\n", response.data);
    } else {
        printf("Unexpected response type: %d\n", response.type);
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
                disconnect(client_fd);
                break;
            case 5:
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
