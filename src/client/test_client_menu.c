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
    char username[MAX_USERNAME];
    printf("Enter username: ");
    scanf("%s", username);

    Message msg = {LOGIN, "", "", "Requesting login"};
    strncpy(msg.username, username, MAX_USERNAME);

    send(client_fd, &msg, sizeof(Message), 0);
    Message response;
    recv(client_fd, &response, sizeof(Message), 0);
    printf("Client received LOGIN response:\n  Type: %d\n  Data: %s\n", response.type, response.data);
}

// Function to handle room creation
void create_room(int client_fd) {
    char room_name[MAX_ROOM_NAME];
    printf("Enter room name: ");
    scanf("%s", room_name);

    Message msg = {CREATE_ROOM, "test_user", "", "Creating room"};
    strncpy(msg.room_name, room_name, MAX_ROOM_NAME);

    send(client_fd, &msg, sizeof(Message), 0);
    Message response;
    recv(client_fd, &response, sizeof(Message), 0);
    printf("Client received CREATE_ROOM response:\n  Type: %d\n  Data: %s\n", response.type, response.data);
}

// Function to handle room joining
void join_room(int client_fd) {
    char room_name[MAX_ROOM_NAME];
    printf("Enter room name to join: ");
    scanf("%s", room_name);

    Message msg = {JOIN_ROOM, "test_user", "", "Joining room"};
    strncpy(msg.room_name, room_name, MAX_ROOM_NAME);

    send(client_fd, &msg, sizeof(Message), 0);
    Message response;
    recv(client_fd, &response, sizeof(Message), 0);
    printf("Client received JOIN_ROOM response:\n  Type: %d\n  Data: %s\n", response.type, response.data);
}

// Function to handle disconnection
void disconnect(int client_fd) {
    Message msg = {DISCONNECT, "test_user", "", "Disconnecting"};
    send(client_fd, &msg, sizeof(Message), 0);
    Message response;
    recv(client_fd, &response, sizeof(Message), 0);
    printf("Client received DISCONNECT response:\n  Type: %d\n  Data: %s\n", response.type, response.data);
}

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
    const char *server_ip = "127.0.0.1";  // Change to your server IP address
    client_function(server_ip);
    return 0;
}
