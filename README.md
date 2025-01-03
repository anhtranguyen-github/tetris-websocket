### Compilation Instructions

#### 1. Create the database


#### 1. Compile the Server
1. Navigate to the server source directory:
   ```bash
   cd src/server
   ```
2. Run the following command to compile the server:
   ```bash
   gcc -o server.o server.c ../ultis.c object.c ../protocol/network.c \
       -I../protocol -I../../config -I../ -I/usr/include/postgresql \
       -L/usr/lib -lpthread -lpq -luuid
   ```
   - **Output**: This will generate an executable file named `server.o` in the `src/server` directory.
3. Run the `server.o` to start the Server

#### 2. Compile the Client
1. Navigate to the client source directory:
   ```bash
   cd src
   ```
2. Run the following command to compile the client test menu:
   ```bash
   gcc main.c tetris_game.c client_utils.c ultis.c -o game.o -lSDL2 -lSDL2_ttf -lpthread
   ```
   - **Output**: This will generate an executable file named `game.o` in the `src` directory.
3. Run the `game.o` file to start the game!

