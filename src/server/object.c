#include "object.h"
#include <string.h>
#include <stdio.h>
#include "../ultis.h"
// Global arrays
//RoomInfo room_infor[MAX_ROOMS];
OnlineGame online_game[MAX_GAME];
OnlineUser online_users[MAX_USERS];


const int ShapesArray[7][4][4] = {
    // Different Tetris shapes
    {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},    // S shape
    {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // Z shape
    {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // T shape
    {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // L shape
    {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // Reverse L shape
    {{1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},    // Square shape
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}     // Long bar shape
};




int generate_random_game_id() {
    srand(time(NULL)); // Seed random generator
    int game_id = rand();
    write_to_log("Generated random game ID:");
    write_to_log_int(game_id);
    return game_id;
}

int is_user_hosting(const char *username) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (online_users[i].user_id != 0 && strcmp(online_users[i].username, username) == 0) {
            return online_users[i].is_hosting;
        }
    }
    return 0; // User not found or not hosting
}


RoomInfo *get_room_info(PGconn *conn, int room_id) {
    char query[BUFFER_SIZE];
    PGresult *result;
    RoomInfo *room_info = malloc(sizeof(RoomInfo));

    if (!room_info) {
        write_to_log("Failed to allocate memory for RoomInfo.");
        return NULL;
    }

    room_info->room_id = room_id;

    // Query to get room information
    snprintf(query, BUFFER_SIZE,
             "SELECT room_name, time_limit, brick_limit, max_players, "
             "(SELECT COUNT(*) FROM room_players WHERE room_id = %d) AS current_players "
             "FROM rooms WHERE room_id = %d;", 
             room_id, room_id);

    write_to_log("Executing query to get room information:");
    write_to_log(query);
    result = PQexec(conn, query);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        write_to_log("Failed to retrieve room information.");
        write_to_log(PQerrorMessage(conn));
        PQclear(result);
        free(room_info);
        return NULL; // Indicate failure
    }

    if (PQntuples(result) == 0) {
        write_to_log("No room found with the given room_id.");
        PQclear(result);
        free(room_info);
        return NULL; // No room found
    }

    // Extract room information
    strncpy(room_info->room_name, PQgetvalue(result, 0, 0), MAX_ROOM_NAME);
    room_info->time_limit = atoi(PQgetvalue(result, 0, 1));
    room_info->brick_limit = atoi(PQgetvalue(result, 0, 2));
    room_info->max_players = atoi(PQgetvalue(result, 0, 3));
    room_info->current_players = atoi(PQgetvalue(result, 0, 4));

    PQclear(result);

    // Query to get usernames of players in the room
    snprintf(query, BUFFER_SIZE,
             "SELECT u.username FROM room_players rp "
             "JOIN users u ON rp.user_id = u.user_id "
             "WHERE rp.room_id = %d;", 
             room_id);

    write_to_log("Executing query to get room players:");
    write_to_log(query);
    result = PQexec(conn, query);

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        write_to_log("Failed to retrieve room players.");
        write_to_log(PQerrorMessage(conn));
        PQclear(result);
        free(room_info);
        return NULL; // Indicate failure
    }

    room_info->room_players[0] = '\0'; // Clear the buffer
    int player_count = PQntuples(result);
    for (int i = 0; i < player_count; i++) {
        if (i > 0) strncat(room_info->room_players, ", ", BUFFER_SIZE - strlen(room_info->room_players) - 1);
        strncat(room_info->room_players, PQgetvalue(result, i, 0), BUFFER_SIZE - strlen(room_info->room_players) - 1);
    }

    write_to_log("Room players fetched successfully:");
    write_to_log(room_info->room_players);
    PQclear(result);

    return room_info; // Return the populated struct
}

// Function to get the brick limit from a RoomInfo struct (no need for DB connection)
int get_brick_limit(RoomInfo *room_info) {
    if (room_info == NULL) {
        return -1; // Indicate an error
    }
    return room_info->brick_limit;
}
int get_room_id_by_username(const char* username) {
    for (int i = 0; i < MAX_USERS; ++i) {
        // Check if the username matches and is not an empty string
        if (online_users[i].username[0] != '\0' && strcmp(online_users[i].username, username) == 0) {
            return online_users[i].room_id;
        }
    }
    return -1; // Return -1 if the username is not found
}
// Function to get the current number of players from a RoomInfo struct (no need for DB connection)
int get_current_players(RoomInfo *room_info) {
    if (room_info == NULL) {
        return -1; // Indicate an error
    }
    return room_info->current_players;
}


// Function to get the list of players from the RoomInfo struct
RoomPlayerList *get_room_players(RoomInfo *room_info) {
    // Check if room_info is valid
    if (room_info == NULL) {
        write_to_log("Invalid RoomInfo provided.");
        return NULL;
    }

    // Create a RoomPlayerList to hold the player names
    RoomPlayerList *player_list = malloc(sizeof(RoomPlayerList));
    if (!player_list) {
        write_to_log("Failed to allocate memory for RoomPlayerList.");
        return NULL;
    }

    // Split room_players (stored in RoomInfo) into individual player names
    // Assuming room_players is a comma-separated list of player names
    char *players_str = room_info->room_players;
    char *player_name;
    int player_count = 0;
    
    // Count the number of players by checking for commas
    char *temp = strdup(players_str);
    char *token = strtok(temp, ", ");
    while (token != NULL) {
        player_count++;
        token = strtok(NULL, ", ");
    }
    free(temp);
    
    player_list->count = player_count;
    player_list->player_names = malloc(player_count * sizeof(char *));
    if (!player_list->player_names) {
        write_to_log("Failed to allocate memory for player names.");
        free(player_list);
        return NULL;
    }

    // Split the player names into the player_names array
    int index = 0;
    token = strtok(players_str, ", ");
    while (token != NULL) {
        player_list->player_names[index] = strdup(token);
        if (!player_list->player_names[index]) {
            write_to_log("Failed to duplicate a player name.");
            for (int i = 0; i < index; i++) {
                free(player_list->player_names[i]);
            }
            free(player_list->player_names);
            free(player_list);
            return NULL;
        }
        index++;
        token = strtok(NULL, ", ");
    }

    write_to_log("Room players fetched successfully.");
    return player_list;
}

// Free function for RoomPlayerList
void free_room_player_list(RoomPlayerList *player_list) {
    if (!player_list) return;
    for (int i = 0; i < player_list->count; i++) {
        free(player_list->player_names[i]);
    }
    free(player_list->player_names);
    free(player_list);
}



Shape copyShape(const int shapeArray[4][4], int width) {
    Shape shape;
    shape.width = width;
    shape.row = 0;
    shape.col = COLS / 2 - width / 2;
    shape.array = malloc(width * sizeof(int*));
    for (int i = 0; i < width; i++) {
        shape.array[i] = malloc(width * sizeof(int));
        for (int j = 0; j < width; j++) {
            shape.array[i][j] = shapeArray[i][j];
        }
    }
    return shape;
}

void freeShape(Shape shape) {
    for (int i = 0; i < shape.width; i++) {
        free(shape.array[i]);
    }
    free(shape.array);
}

void generateShapes(ShapeList *shapeList, int number) {
    for (int i = 0; i < number; i++) {
        int index = rand() % 7;
        Shape newShape = copyShape(ShapesArray[index], 4);
        if (shapeList->count < MAX_SHAPES) {
            shapeList->shapes[shapeList->count++] = newShape;
        } else {
            freeShape(newShape); // Avoid memory leak if list is full
        }
    }
}

int* generateShapesInt(int number) {
    // Log the function entry and the requested number of shapes
    printf("generateShapesInt: Requested number of shapes: %d\n", number);

    // Allocate memory for the shape list
    int *shapeList = (int*)malloc(number * sizeof(int));
    if (shapeList == NULL) {
        // Log memory allocation failure
        printf("generateShapesInt: Memory allocation failed\n");
        return NULL;
    }
    printf("generateShapesInt: Memory allocation successful\n");

    // Populate the shape list with random integers between 0 and 6
    for (int i = 0; i < number; i++) {
        shapeList[i] = rand() % 7;
        // Log each generated shape
        printf("generateShapesInt: shapeList[%d] = %d\n", i, shapeList[i]);
    }

    // Log the successful completion of the function
    printf("generateShapesInt: Shape list generation completed\n");
    return shapeList;
}


void freeShapeList(ShapeList *shapeList) {
    for (int i = 0; i < shapeList->count; i++) {
        freeShape(shapeList->shapes[i]);
    }
    shapeList->count = 0;
}

// Serialize the ShapeList to a string
char* serializeShapeList(ShapeList *shapeList) {
    char *buffer = malloc(BUFFER_SIZE);
    buffer[0] = '\0';
    char temp[256];

    sprintf(temp, "%d,%d\n", shapeList->count, shapeList->current);
    strcat(buffer, temp);

    for (int i = 0; i < shapeList->count; i++) {
        Shape *shape = &shapeList->shapes[i];
        sprintf(temp, "%d,%d,%d\n", shape->width, shape->row, shape->col);
        strcat(buffer, temp);
        for (int r = 0; r < shape->width; r++) {
            for (int c = 0; c < shape->width; c++) {
                sprintf(temp, "%d", shape->array[r][c]);
                strcat(buffer, temp);
                if (c < shape->width - 1) strcat(buffer, ",");
            }
            strcat(buffer, "\n");
        }
    }
    return buffer;
}


char* serializeShapeListInt(int *shapeList, int count) {
    char *buffer = (char*)malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        // Handle memory allocation failure
        return NULL;
    }
    buffer[0] = '\0'; // Initialize the buffer as an empty string

    char temp[32];
    sprintf(temp, "%d\n", count); // Add the count as the first line
    strcat(buffer, temp);

    for (int i = 0; i < count; i++) {
        sprintf(temp, "%d", shapeList[i]); // Convert the integer to string
        strcat(buffer, temp); // Append the integer to the buffer
        if (i < count - 1) {
            strcat(buffer, ","); // Add a comma separator between elements
        }
    }
    strcat(buffer, "\n"); // Add a newline at the end

    return buffer;
}
// Deserialize a string into a ShapeList
void deserializeShapeList(ShapeList *shapeList, const char *data) {
    freeShapeList(shapeList); // Ensure the list is empty before loading new data

    const char *line = data;
    int width, row, col;
    sscanf(line, "%d,%d\n", &shapeList->count, &shapeList->current);
    line = strchr(line, '\n') + 1;

    for (int i = 0; i < shapeList->count; i++) {
        sscanf(line, "%d,%d,%d\n", &width, &row, &col);
        line = strchr(line, '\n') + 1;

        Shape shape;
        shape.width = width;
        shape.row = row;
        shape.col = col;
        shape.array = malloc(width * sizeof(int*));
        for (int r = 0; r < width; r++) {
            shape.array[r] = malloc(width * sizeof(int));
            for (int c = 0; c < width; c++) {
                sscanf(line, "%d,", &shape.array[r][c]);
                line = strchr(line, c < width - 1 ? ',' : '\n') + 1;
            }
        }
        shapeList->shapes[i] = shape;
    }
}





void initLeaderboard(Leaderboard *leaderboard, RoomPlayerList *player_list) {
    leaderboard->current_players = 0;  // Start with no scores

    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        leaderboard->entries[i].name[0] = '\0';
        leaderboard->entries[i].score = 0;
    }

    // Initialize leaderboard entries based on player_list
    for (int i = 0; i < player_list->count && i < LEADERBOARD_SIZE; i++) {
        strncpy(leaderboard->entries[i].name, player_list->player_names[i], MAX_USERNAME - 1);
        leaderboard->entries[i].name[MAX_USERNAME - 1] = '\0';
        leaderboard->entries[i].score = 0;  // Start all scores at 0
        leaderboard->current_players++;
    }
}

// Add or update a player in the leaderboard
void update_leaderboard(Leaderboard *leaderboard, const char *player, int points) {
    // Check if the player already exists
    for (int i = 0; i < leaderboard->current_players; i++) {
        if (strncmp(leaderboard->entries[i].name, player, MAX_USERNAME) == 0) {
            leaderboard->entries[i].score += points; // Update existing player's score
            return;
        }
    }

    // Add new player if space is available
    if (leaderboard->current_players < LEADERBOARD_SIZE) {
        strncpy(leaderboard->entries[leaderboard->current_players].name, player, MAX_USERNAME - 1);
        leaderboard->entries[leaderboard->current_players].name[MAX_USERNAME - 1] = '\0';
        leaderboard->entries[leaderboard->current_players].score = points;
        leaderboard->current_players++;
    } else {
        printf("Leaderboard is full! Cannot add player: %s\n", player);
    }
}

// Serialize the leaderboard to a string
char* serializeLeaderboard(const Leaderboard *leaderboard) {
    char *buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    buffer[0] = '\0'; // Initialize buffer

    char temp[64];
    snprintf(temp, sizeof(temp), "%d\n", leaderboard->current_players);
    strncat(buffer, temp, BUFFER_SIZE - strlen(buffer) - 1);

    for (int i = 0; i < leaderboard->current_players; i++) {
        snprintf(temp, sizeof(temp), "%s,%d\n", leaderboard->entries[i].name, leaderboard->entries[i].score);
        strncat(buffer, temp, BUFFER_SIZE - strlen(buffer) - 1);
    }

    return buffer;
}

// Deserialize a string into a leaderboard
void deserializeLeaderboard(Leaderboard *leaderboard, const char *data) {
    initLeaderboard(leaderboard, &(RoomPlayerList){NULL, 0}); // Reset the leaderboard

    const char *line = data;
    sscanf(line, "%d\n", &leaderboard->current_players);
    line = strchr(line, '\n');
    if (!line) return; // Exit if malformed input
    line++; // Move to the next line

    for (int i = 0; i < leaderboard->current_players && i < LEADERBOARD_SIZE; i++) {
        if (sscanf(line, "%31[^,],%d\n", leaderboard->entries[i].name, &leaderboard->entries[i].score) != 2) {
            printf("Malformed entry found, stopping deserialization.\n");
            break;
        }
        line = strchr(line, '\n');
        if (!line) break; // Prevent null dereference
        line++;
    }
}



// Initialize the online_game array
void init_online_games() {
    for (int i = 0; i < MAX_GAME; i++) {
        online_game[i].game_id = -1; // Mark all slots as unused
    }
}

// Find an empty slot in the online_game array
int find_empty_game_slot() {
    for (int i = 0; i < MAX_GAME; i++) {
        if (online_game[i].game_id == -1) {
            return i; // Found an empty slot
        }
    }
    return -1; // No empty slots
}


void create_online_game(PGconn *conn, int room_id) {
    write_to_log("Starting create_online_game");

    // Find an empty slot in the online_game array
    int slot = find_empty_game_slot();
    if (slot == -1) {
        write_to_log("No available slots for a new game. Exiting function.");
        return;
    }

    // Generate random game ID
    int game_id = generate_random_game_id();
    char log_message[256];
    snprintf(log_message, sizeof(log_message), "Generated Game ID: %d", game_id);
    write_to_log(log_message);

    // Get room info
    RoomInfo *room_info = get_room_info(conn, room_id);
    if (room_info == NULL) {
        write_to_log("Failed to get room info. Exiting function.");
        return;
    }
    write_to_log("Got room info");

    // Get brick limit
    int brick_limit = get_brick_limit(room_info);
    snprintf(log_message, sizeof(log_message), "Brick Limit: %d", brick_limit);
    write_to_log(log_message);

    // Get room players
    RoomPlayerList *player_list = get_room_players(room_info);
    if (player_list == NULL) {
        write_to_log("Failed to get room players. Exiting function.");
        free(room_info);
        return;
    }

    // Debugging the RoomPlayerList
    write_to_log("Got room players");
    snprintf(log_message, sizeof(log_message), "Player Count: %d", player_list->count);
    write_to_log(log_message);
    for (int i = 0; i < player_list->count; i++) {
        if (player_list->player_names[i] == NULL) {
            snprintf(log_message, sizeof(log_message), "Error: player name at index %d is NULL", i);
            write_to_log(log_message);
        } else {
            snprintf(log_message, sizeof(log_message), "  Player %d: %s", i + 1, player_list->player_names[i]);
            write_to_log(log_message);
        }
    }

    // Generate shapes
    int *shape_list = generateShapesInt(brick_limit);
    if (shape_list == NULL) {
        write_to_log("Failed to generate shapes. Exiting function.");
        for (int i = 0; i < player_list->count; i++) {
            free(player_list->player_names[i]);
        }
        free(player_list->player_names);
        free(player_list);
        free(room_info);
        return;
    }
    write_to_log("Generated shapes");

    // Initialize leaderboard
    Leaderboard leaderboard;
    initLeaderboard(&leaderboard, player_list);
    write_to_log("Initialized leaderboard");

    // Store the game in the online_game array
    online_game[slot].game_id = game_id;
    online_game[slot].room_id = room_id;
    online_game[slot].brick_limit = brick_limit;
    online_game[slot].shapeList = shape_list;
    online_game[slot].leaderboard = leaderboard;
    write_to_log("Game added to online_game array");

    // Cleanup
    for (int i = 0; i < player_list->count; i++) {
        free(player_list->player_names[i]);
    }
    free(player_list->player_names);
    free(player_list);
    free(room_info);
    write_to_log("Online game created successfully.");
}


char* serializeOnlineGame(const OnlineGame *online_game) {
    if (online_game == NULL) {
        write_to_log("serializeOnlineGame: online_game is NULL");
        return NULL; // Handle null input
    }
    write_to_log("serializeOnlineGame: online_game is not NULL");

    // Allocate buffer for serialization
    char *buffer = (char*)malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        write_to_log("serializeOnlineGame: Memory allocation failed for buffer");
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    buffer[0] = '\0'; // Initialize buffer as empty string
    write_to_log("serializeOnlineGame: Buffer allocated and initialized");

    // Serialize room_id and game_id
    char temp[BUFFER_SIZE];
    snprintf(temp, sizeof(temp), "Room ID: %d\nGame ID: %d\n", online_game->room_id, online_game->game_id);
    strncat(buffer, temp, BUFFER_SIZE - strlen(buffer) - 1);
    write_to_log("serializeOnlineGame: Serialized room_id and game_id");

    // Serialize shapeList


    char *shapeListStr = serializeShapeListInt(online_game->shapeList, online_game->brick_limit);
    if (shapeListStr != NULL) {
        strncat(buffer, "Shapes:\n", BUFFER_SIZE - strlen(buffer) - 1);
        strncat(buffer, shapeListStr, BUFFER_SIZE - strlen(buffer) - 1);
        free(shapeListStr);
        write_to_log("serializeOnlineGame: Serialized shapeList successfully");
    } else {
        write_to_log("serializeOnlineGame: Failed to serialize shapeList");
    }
    // Serialize leaderboard
    char *leaderboardStr = serializeLeaderboard(&(online_game->leaderboard));
    if (leaderboardStr != NULL) {
        strncat(buffer, "Leaderboard:\n", BUFFER_SIZE - strlen(buffer) - 1);
        strncat(buffer, leaderboardStr, BUFFER_SIZE - strlen(buffer) - 1);
        free(leaderboardStr);
        write_to_log("serializeOnlineGame: Serialized leaderboard");
    } else {
        write_to_log("serializeOnlineGame: serializeLeaderboard returned NULL");
    }

    write_to_log("serializeOnlineGame: Serialization complete");
    return buffer;
}


Message create_start_game_message(const OnlineGame *game) {
    // Initialize the message
    Message message;
    memset(&message, 0, sizeof(Message)); // Ensure all fields are zeroed out

    // Set message type
    message.type = START_GAME_SUCCESS;

    // Populate username and room_name (these can be placeholders if not applicable)
    snprintf(message.username, MAX_USERNAME, "System"); // Example: System message
    snprintf(message.room_name, MAX_ROOM_NAME, "%d", game->room_id);

    // Serialize the game data into the `data` field
    char *serialized_game = serializeOnlineGame(game);
    if (serialized_game != NULL) {
        strncpy(message.data, serialized_game, BUFFER_SIZE - 1); // Ensure null-termination
        message.data[BUFFER_SIZE - 1] = '\0';                   // Safety null-termination
        free(serialized_game); // Free allocated memory for serialization
    } else {
        write_to_log("create_start_game_message: Failed to serialize game data");
        strncpy(message.data, "Failed to serialize game data", BUFFER_SIZE - 1);
        message.data[BUFFER_SIZE - 1] = '\0';
    }

    // Log the creation of the message
    write_to_log("create_start_game_message: Message created successfully");
    return message;
}
ShapeList shapeList;

void get_shape_list_int(int* shapeList, const Message* message) {
    if (message == NULL || shapeList == NULL) {
        write_to_log("get_shape_list_int: Invalid input parameters");
        return;
    }

    // Find the "Shapes:" section in the message data
    const char *shapes_section = strstr(message->data, "Shapes:\n");
    if (shapes_section == NULL) {
        write_to_log("get_shape_list_int: Shapes section not found in message data");
        return;
    }

    // Move the pointer to the start of the shape list length
    shapes_section += strlen("Shapes:\n");

    // Extract the count of shapes
    int count;
    sscanf(shapes_section, "%d\n", &count);

    // Move the pointer to the start of the shape list data
    const char *shape_data = strchr(shapes_section, '\n') + 1;

    // Extract the shape list integers
    for (int i = 0; i < count; i++) {
        sscanf(shape_data, "%d", &shapeList[i]);
        shape_data = strchr(shape_data, ',');
        if (shape_data != NULL) {
            shape_data++;
        }
    }

    // Add -1 at the end of the shape list
    shapeList[count] = -1;

    write_to_log("get_shape_list_int: Shape list extracted successfully");
}


void convertIntToShapeList(int* shapeListInt) {
    shapeList.count = 0;
    shapeList.current = 0;

    int i = 0;
    while (shapeListInt[i] != -1) {
        Shape shape;
        shape.width = 4;
        shape.row = 4;
        shape.col = 4;
        shape.array = malloc(4 * sizeof(int*));
        for (int j = 0; j < 4; j++) {
            shape.array[j] = malloc(4 * sizeof(int));
            for (int k = 0; k < 4; k++) {
                shape.array[j][k] = ShapesArray[shapeListInt[i]][j][k];
            }
        }
        shapeList.shapes[shapeList.count++] = shape;
        i++;
    }
}
