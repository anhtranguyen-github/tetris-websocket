# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket

# Include any dependencies generated for this target.
include CMakeFiles/tetris_offline.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/tetris_offline.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/tetris_offline.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tetris_offline.dir/flags.make

CMakeFiles/tetris_offline.dir/src/main.c.o: CMakeFiles/tetris_offline.dir/flags.make
CMakeFiles/tetris_offline.dir/src/main.c.o: src/main.c
CMakeFiles/tetris_offline.dir/src/main.c.o: CMakeFiles/tetris_offline.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/tetris_offline.dir/src/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/tetris_offline.dir/src/main.c.o -MF CMakeFiles/tetris_offline.dir/src/main.c.o.d -o CMakeFiles/tetris_offline.dir/src/main.c.o -c /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/src/main.c

CMakeFiles/tetris_offline.dir/src/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/tetris_offline.dir/src/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/src/main.c > CMakeFiles/tetris_offline.dir/src/main.c.i

CMakeFiles/tetris_offline.dir/src/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/tetris_offline.dir/src/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/src/main.c -o CMakeFiles/tetris_offline.dir/src/main.c.s

CMakeFiles/tetris_offline.dir/src/tetris_game.c.o: CMakeFiles/tetris_offline.dir/flags.make
CMakeFiles/tetris_offline.dir/src/tetris_game.c.o: src/tetris_game.c
CMakeFiles/tetris_offline.dir/src/tetris_game.c.o: CMakeFiles/tetris_offline.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/tetris_offline.dir/src/tetris_game.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/tetris_offline.dir/src/tetris_game.c.o -MF CMakeFiles/tetris_offline.dir/src/tetris_game.c.o.d -o CMakeFiles/tetris_offline.dir/src/tetris_game.c.o -c /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/src/tetris_game.c

CMakeFiles/tetris_offline.dir/src/tetris_game.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/tetris_offline.dir/src/tetris_game.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/src/tetris_game.c > CMakeFiles/tetris_offline.dir/src/tetris_game.c.i

CMakeFiles/tetris_offline.dir/src/tetris_game.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/tetris_offline.dir/src/tetris_game.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/src/tetris_game.c -o CMakeFiles/tetris_offline.dir/src/tetris_game.c.s

# Object files for target tetris_offline
tetris_offline_OBJECTS = \
"CMakeFiles/tetris_offline.dir/src/main.c.o" \
"CMakeFiles/tetris_offline.dir/src/tetris_game.c.o"

# External object files for target tetris_offline
tetris_offline_EXTERNAL_OBJECTS =

bin/tetris_offline: CMakeFiles/tetris_offline.dir/src/main.c.o
bin/tetris_offline: CMakeFiles/tetris_offline.dir/src/tetris_game.c.o
bin/tetris_offline: CMakeFiles/tetris_offline.dir/build.make
bin/tetris_offline: CMakeFiles/tetris_offline.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable bin/tetris_offline"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tetris_offline.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tetris_offline.dir/build: bin/tetris_offline
.PHONY : CMakeFiles/tetris_offline.dir/build

CMakeFiles/tetris_offline.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tetris_offline.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tetris_offline.dir/clean

CMakeFiles/tetris_offline.dir/depend:
	cd /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/CMakeFiles/tetris_offline.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/tetris_offline.dir/depend

