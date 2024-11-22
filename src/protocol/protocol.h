#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PORT 12345
#define MAX_CLIENTS 10
#define MAX_ROOMS 5
#define MAX_ROOM_NAME 32
#define MAX_USERNAME 32
#define BUFFER_SIZE 256

// Message types
typedef enum {
    LOGIN, LOGIN_SUCCESS, LOGIN_FAILURE,
    CREATE_ROOM,
    JOIN_ROOM, JOIN_ROOM_SUCESS, JOIN_ROOM_FAILURE,
    ROOM_LIST,
    ROOM_JOINED,
    ROOM_NOT_FOUND,
    GAME_START,
    DISCONNECT
} MessageType;

// Message structure
typedef struct {
    MessageType type;
    char username[MAX_USERNAME];
    char room_name[MAX_ROOM_NAME];
    char data[BUFFER_SIZE];
} Message;

#endif
