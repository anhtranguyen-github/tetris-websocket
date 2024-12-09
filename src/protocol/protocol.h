#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PORT 12345
#define MAX_CLIENTS 10
#define MAX_ROOMS 5
#define MAX_ROOM_NAME 32
#define MAX_USERNAME 32
#define BUFFER_SIZE 1000
#define MAX_SESSION_ID 40

// Message types
typedef enum {
    LOGIN, LOGIN_SUCCESS, LOGIN_FAILURE,
    CREATE_ROOM, CREATE_ROOM_SUCCESS, CREATE_ROOM_FAILURE,
    JOIN_ROOM, JOIN_ROOM_SUCCESS, JOIN_ROOM_FAILURE, ROOM_NOT_FOUND, ROOM_FULL, GAME_ALREADY_STARTED, PLAYER_JOINED, JOIN_RANDOM,
    ROOM_LIST,
    ROOM_JOINED,
    GAME_START,
    GAME_STATUS,
    DISCONNECT,
// Broadcast when a new player joins
} MessageType;
// Message structure
typedef struct {
    MessageType type;
    char username[MAX_USERNAME];
    char room_name[MAX_ROOM_NAME];
    char data[BUFFER_SIZE];
} Message;

#endif
