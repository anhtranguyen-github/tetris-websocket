### Compilation Instructions

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
   - **Output**: This will generate an executable file named `server` in the `src/server` directory.

#### 2. Compile the Client
1. Navigate to the client source directory:
   ```bash
   cd src/client
   ```
2. Run the following command to compile the client test menu:
   ```bash
   gcc -o test_client_menu test_client_menu.c ../ultis.c \
       -I../protocol -I../../config -lpthread
   ```
   - **Output**: This will generate an executable file named `test_client_menu` in the `src/client` directory.

