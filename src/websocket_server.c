#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "tetris_game.h"

#define PORT 12345
#define MAX_CLIENTS 4
#define BLOCKS_TO_SEND 20

typedef struct {
    int socket;
    char name[50];
    int score;
    int active;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
int server_socket;

void initServer() {
    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
}

void addClient(int client_socket) {
    if (client_count < MAX_CLIENTS) {
        clients[client_count].socket = client_socket;
        clients[client_count].score = 0;
        clients[client_count].active = 1;
        client_count++;
    } else {
        printf("Maximum clients reached. Cannot add more.\n");
        close(client_socket);
    }
}

void broadcastMessage(const char *message) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].active) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

void sendBlockToAllClients(int block_index) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].active) {
            send(clients[i].socket, &block_index, sizeof(block_index), 0);
        }
    }
}

void collectScoresFromClients() {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].active) {
            recv(clients[i].socket, &clients[i].score, sizeof(int), 0);
        }
    }
}

/*
void declareWinner() {
    int max_score = -1;
    char winner[50] = "No one";

    for (int i = 0; i < client_count; i++) {
        if (clients[i].score > max_score) {
            max_score = clients[i].score;
            strcpy(winner, clients[i].name);
        }
    }

    char result[100];
    snprintf(result, sizeof(result), "Game Over! Winner: %s with %d points.\n", winner, max_score);
    broadcastMessage(result);
}
*/

int main() {
    initServer();

    fd_set read_fds;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        int max_sd = server_socket;

        // Add all client sockets to the read_fds set
        for (int i = 0; i < client_count; i++) {
            if (clients[i].active) {
                FD_SET(clients[i].socket, &read_fds);
                if (clients[i].socket > max_sd) {
                    max_sd = clients[i].socket;
                }
            }
        }

        // Wait for activity on one of the sockets
        int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("select error");
        }

        // Check if there's a new incoming connection
        if (FD_ISSET(server_socket, &read_fds)) {
            int new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
            if (new_socket == -1) {
                perror("Accept failed");
                continue;
            }

            printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Ask for the client's name
            send(new_socket, "Enter your name: ", strlen("Enter your name: "), 0);
            char client_name[50];
            recv(new_socket, client_name, sizeof(client_name), 0);

            // Add the client to the list
            addClient(new_socket);
            strcpy(clients[client_count - 1].name, client_name);
            printf("Client %s added.\n", client_name);

            // If we have at least 2 clients, start the game
            if (client_count >= 2) {
                broadcastMessage("Game starting...\n");
                srand(time(NULL));

                // Generate and send 20 blocks
                for (int i = 0; i < BLOCKS_TO_SEND; i++) {
                    int block_index = rand() % 7;
                    sendBlockToAllClients(block_index);
                }

                // Collect scores and declare winner
                collectScoresFromClients();
                declareWinner();
            }
        }

        // Check for messages from each client
        for (int i = 0; i < client_count; i++) {
            if (clients[i].active && FD_ISSET(clients[i].socket, &read_fds)) {
                char buffer[1024];
                int bytes_read = recv(clients[i].socket, buffer, sizeof(buffer), 0);
                if (bytes_read <= 0) {
                    // Client disconnected
                    printf("Client %s disconnected.\n", clients[i].name);
                    clients[i].active = 0;
                    close(clients[i].socket);
                } else {
                    buffer[bytes_read] = '\0';
                    printf("Received from %s: %s\n", clients[i].name, buffer);
                }
            }
        }
    }

    close(server_socket);
    return 0;
}
