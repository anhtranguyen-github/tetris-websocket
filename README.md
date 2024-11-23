
# Tetris Game with WebSocket and Multithreading

This project is a Tetris game built using C, SDL2, and WebSocket for real-time multiplayer functionality. It includes support for SDL2 graphics, multithreading with `pthread`, and a client-server architecture where players can connect and compete against each other.

## Features
- **Tetris Gameplay**: A fully functional Tetris game with the classic gameplay mechanics.
- **Multithreading**: Server and client support multithreading for concurrent handling of user actions and game state updates.
- **WebSocket Server**: A WebSocket-based server to facilitate communication between multiple players in real time.
- **SDL2**: Graphics and UI for the game using SDL2, including support for custom fonts and backgrounds.

## Project Structure

```
.
├── CMakeLists.txt
├── README.md
├── assets
├── build_project.sh
├── config
│   ├── client_config.h
│   └── server_config.h
├── design
└── src
    ├── client
    │   ├── client.c
    │   ├── client.h
    │   ├── sdl_utils.c
    │   ├── sdl_utils.h
    │   └── test_client_menu.c
    ├── db
    │   └── db.c
    ├── main.c
    ├── protocol
    │   ├── network.c
    │   ├── network.h
    │   └── protocol.h
    ├── server
    │   ├── server.c
    │   └── server.h
    ├── tetris_game.c
    ├── tetris_game.h
    ├── ultis.c
    └── ultis.h
```

## Requirements

- **C Compiler**: GCC or any compatible C compiler.
- **CMake**: Version 3.10 or later for building the project.
- **SDL2**: SDL2 graphics library for rendering.
- **SDL2_ttf**: SDL2 extension for working with TrueType fonts.
- **pthread**: For multithreading support.
- **pkg-config**: For handling library dependencies.
- **PostgreSQL (psql)**: For managing the database.

## Building the Project

### 1. Clone the Repository

First, clone the repository to your local machine:

```bash
git clone https://github.com/your-username/tetris-websocket.git
cd tetris-websocket
```

### 2. Install Dependencies (NEED TO BE CHECKED !!!)

Ensure that you have SDL2, SDL2_ttf, and pthread installed. On Ubuntu, you can install them using the following commands:

```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-ttf-dev libpthread-stubs0-dev postgresql postgresql-client
```

### 3. Build the Project

Run the `build_project.sh` script to automate the build process:

```bash
./build_project.sh
```

This will:
- Create the `build` directory if it doesn't exist.
- Run `cmake` to configure the project.
- Build the project using `make`.

### Outputs

The following executables will be created in the `build/bin/` directory:
- **db**: Creates a database in PostgreSQL.
- **server**: Initializes the server.
- **test_client_menu**: Provides a client menu in the console command line.

## Running the Game

1. **Start the server**:

   ```bash
   ./bin/server
   ```

2. **Start the client**:

   ```bash
   ./bin/test_client_menu
   ```

## Customization

- **Fonts**: The game uses custom fonts from the `assets/fonts` folder.
- **Server Configuration**: Adjust server parameters in the `config/server_config.h` file.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
