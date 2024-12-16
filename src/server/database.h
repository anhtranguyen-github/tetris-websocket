#ifndef DATABASE_H
#define DATABASE_H

#include <libpq-fe.h>

// Function prototypes
void create_game(PGconn *conn, int room_id);
void update_game_status(PGconn *conn, int game_id, int status);

#endif 
