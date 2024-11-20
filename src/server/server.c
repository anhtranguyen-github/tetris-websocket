#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "../protocol/network.c"
#include "../protocol/protocol.h"
#include <signal.h>
// Function to simulate random login response
Message handle_login(const Message *msg) {
    Message response;
    response.type = rand() % 2 + 1;  // Randomly choose between LOGIN, LOGIN_SUCCESS, LOGIN_FAILURE

    switch (response.type) {
        case LOGIN_SUCCESS:
            strcpy(response.data, "Login successful!");
            break;
        case LOGIN_FAILURE:
            strcpy(response.data, "Login failed!");
            break;
        default:
            break;
    }
    return response;
}

// Function to simulate random room join response
Message handle_join_room(const Message *msg) {
    Message response;
    response.type = rand() % 2 + 6;  // Randomly choose between ROOM_JOINED, ROOM_NOT_FOUND

    switch (response.type) {
        case ROOM_JOINED:
            strcpy(response.data, "Room joined successfully!");
            break;
        case ROOM_NOT_FOUND:
            strcpy(response.data, "Room not found!");
            break;
        default:
            break;
    }
    return response;
}

// Function to handle client requests
void handle_client(int client_fd) {
    Message msg, response;
    
    while (recv(client_fd, &msg, sizeof(Message), 0) > 0) {
        printf("Server received message of type %d from %s\n", msg.type, msg.username);
        
        switch (msg.type) {
            case LOGIN:
                response = handle_login(&msg);
                break;
            case CREATE_ROOM:
                // Create room logic here
                strcpy(response.data, "Room created successfully!");
                response.type = ROOM_LIST;  // Example, room list can be sent back
                break;
            case JOIN_ROOM:
                response = handle_join_room(&msg);
                break;
            case DISCONNECT:
                strcpy(response.data, "Disconnected from server.");
                response.type = DISCONNECT;
                break;
            default:
                strcpy(response.data, "Unknown message type.");
                response.type = -1;  // Unknown message type
        }
        
        // Send the response back to the client
        send(client_fd, &response, sizeof(Message), 0);
    }

    close(client_fd);
}

// Thread function to handle client connection
void *client_thread(void *arg) {
    int client_fd = *((int *)arg);
    free(arg);
    handle_client(client_fd);
    return NULL;
}






int server_fd;

// Signal handler to clean up and close the server socket
void handle_signal(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    if (server_fd > 0) {
        close(server_fd);
    }
    exit(0);
}

int main() {


    signal(SIGINT, handle_signal);  // Handle Ctrl+C
    signal(SIGTERM, handle_signal); 

    int server_fd = create_server_socket();
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t tid;
        int *client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;
        if (pthread_create(&tid, NULL, client_thread, client_fd_ptr) != 0) {
            perror("Failed to create thread");
            free(client_fd_ptr);
        }
    }

    close(server_fd);
    return 0;
}
