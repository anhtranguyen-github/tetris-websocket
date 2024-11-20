
# Tetris Game with WebSocket and Multithreading

This project is a Tetris game built using C, SDL2, and WebSocket for real-time multiplayer functionality. It includes support for SDL2 graphics, multithreading with `pthread`, and a client-server architecture where players can connect and compete against each other.

## Features
- **Tetris Gameplay**: A fully functional Tetris game with the classic gameplay mechanics.
- **Multithreading**: Server and client support multithreading for concurrent handling of user actions and game state updates.
- **WebSocket Server**: A WebSocket-based server to facilitate communication between multiple players in real-time.
- **SDL2**: Graphics and UI for the game using SDL2, including support for custom fonts and backgrounds.

## Project Structure

```
.
├── CMakeLists.txt
├── README.md
├── assets
├── build
├── build_project.sh
├── config
│   └── server_config.h
├── design
│   ├── PvP Sequence.asta
│   ├── PvP Sequence.asta.lock
│   └── PvP Sequence.png
├── src
│   ├── client
│   │   ├── client.c
│   │   ├── client.h
│   │   ├── sdl_utils.c
│   │   ├── sdl_utils.h
│   │   └── test_client_menu.c
│   ├── db
│   │   └── db.c
│   ├── main.c
│   ├── protocol
│   │   ├── network.c
│   │   ├── network.h
│   │   └── protocol.h
│   ├── server
│   │   ├── server
│   │   ├── server.c
│   │   └── server.h
│   ├── tetris_game.c
│   └── tetris_game.h

```

- `assets/` contains images and fonts used for the game's UI.
- `config/` contains configuration files.
- `src/` contains all the source files for the client, server, game logic, and protocol handling.
- `CMakeLists.txt` is the build configuration file for CMake.
- `README.md` is this file.

## Requirements

- **C Compiler**: GCC or any compatible C compiler.
- **CMake**: Version 3.10 or later for building the project.
- **SDL2**: SDL2 graphics library for rendering.
- **SDL2_ttf**: SDL2 extension for working with TrueType fonts.
- **pthread**: For multithreading support.
- **pkg-config**: For handling library dependencies.

## Building the Project

### 1. Clone the Repository

First, clone the repository to your local machine:

```bash
git clone https://github.com/your-username/tetris-websocket.git
cd tetris-websocket
```

### 2. Install Dependencies (NEED TO CHECK !!!)

Ensure that you have SDL2, SDL2_ttf, and pthread installed. On Ubuntu, you can install them using the following commands:

```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-ttf-dev libpthread-stubs0-dev
```

### 3. Build the Project with CMake

To build the project, follow these steps:

1. **Run the build script**:

   To automate the process of creating the build directory, configuring the project, and building it, simply run the `build_project.sh` script:

   ```bash
   ./build_project.sh
   ```

   This will:
   - Create the `build` directory if it doesn't exist.
   - Run `cmake` to configure the project.
   - Build the project using `make`.

2. **Alternatively, manually build the project**:

   If you prefer to manually set up the build environment, follow these steps:

   - Create a build directory:

     ```bash
     mkdir build
     cd build
     ```

   - Run CMake to configure the project:

     ```bash
     cmake ..
     ```

   - Build the project using `make`:

     ```bash
     make
     ```

### 4. Build Specific Targets

You can build the specific components of the project (server, test client, and Tetris offline game) by running:

```bash
make server
make test_client_menu
make tetris_offline
```

### 5. Run the Game

Once the project is successfully built, you can run the Tetris game with the following command:

```bash
./tetris_offline
```

To run the server and client, use the following commands:

1. **Start the server**:

   ```bash
   ./bin/server 
   ```

2. **Start the client**:

   ```bash
   ./bin/test_client_menu 
   ```


## Usage

- The **server** manages the game state and handles communication between multiple clients.
- The **client** connects to the server, allowing a player to interact with the game.

### Client-Server Communication

- The game uses WebSocket for real-time communication between the client and server.
- The server uses multithreading to handle multiple client connections simultaneously.

## Customization

- **Fonts**: The game uses custom fonts from the `assets/fonts` folder.
- **Server Configuration**: Adjust server parameters in the `config/server_config.h` file.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
