# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

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

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/CMakeFiles /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /mnt/c/Users/tra01/OneDrive/Desktop/VSC/tetris/tetris-websocket/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named server

# Build rule for target.
server: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 server
.PHONY : server

# fast build rule for target.
server/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/build
.PHONY : server/fast

#=============================================================================
# Target rules for targets named test_client_menu

# Build rule for target.
test_client_menu: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 test_client_menu
.PHONY : test_client_menu

# fast build rule for target.
test_client_menu/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_client_menu.dir/build.make CMakeFiles/test_client_menu.dir/build
.PHONY : test_client_menu/fast

#=============================================================================
# Target rules for targets named tetris_offline

# Build rule for target.
tetris_offline: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 tetris_offline
.PHONY : tetris_offline

# fast build rule for target.
tetris_offline/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tetris_offline.dir/build.make CMakeFiles/tetris_offline.dir/build
.PHONY : tetris_offline/fast

src/client/test_client_menu.o: src/client/test_client_menu.c.o
.PHONY : src/client/test_client_menu.o

# target to build an object file
src/client/test_client_menu.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_client_menu.dir/build.make CMakeFiles/test_client_menu.dir/src/client/test_client_menu.c.o
.PHONY : src/client/test_client_menu.c.o

src/client/test_client_menu.i: src/client/test_client_menu.c.i
.PHONY : src/client/test_client_menu.i

# target to preprocess a source file
src/client/test_client_menu.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_client_menu.dir/build.make CMakeFiles/test_client_menu.dir/src/client/test_client_menu.c.i
.PHONY : src/client/test_client_menu.c.i

src/client/test_client_menu.s: src/client/test_client_menu.c.s
.PHONY : src/client/test_client_menu.s

# target to generate assembly for a file
src/client/test_client_menu.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/test_client_menu.dir/build.make CMakeFiles/test_client_menu.dir/src/client/test_client_menu.c.s
.PHONY : src/client/test_client_menu.c.s

src/main.o: src/main.c.o
.PHONY : src/main.o

# target to build an object file
src/main.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tetris_offline.dir/build.make CMakeFiles/tetris_offline.dir/src/main.c.o
.PHONY : src/main.c.o

src/main.i: src/main.c.i
.PHONY : src/main.i

# target to preprocess a source file
src/main.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tetris_offline.dir/build.make CMakeFiles/tetris_offline.dir/src/main.c.i
.PHONY : src/main.c.i

src/main.s: src/main.c.s
.PHONY : src/main.s

# target to generate assembly for a file
src/main.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tetris_offline.dir/build.make CMakeFiles/tetris_offline.dir/src/main.c.s
.PHONY : src/main.c.s

src/server/server.o: src/server/server.c.o
.PHONY : src/server/server.o

# target to build an object file
src/server/server.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/src/server/server.c.o
.PHONY : src/server/server.c.o

src/server/server.i: src/server/server.c.i
.PHONY : src/server/server.i

# target to preprocess a source file
src/server/server.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/src/server/server.c.i
.PHONY : src/server/server.c.i

src/server/server.s: src/server/server.c.s
.PHONY : src/server/server.s

# target to generate assembly for a file
src/server/server.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/server.dir/build.make CMakeFiles/server.dir/src/server/server.c.s
.PHONY : src/server/server.c.s

src/tetris_game.o: src/tetris_game.c.o
.PHONY : src/tetris_game.o

# target to build an object file
src/tetris_game.c.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tetris_offline.dir/build.make CMakeFiles/tetris_offline.dir/src/tetris_game.c.o
.PHONY : src/tetris_game.c.o

src/tetris_game.i: src/tetris_game.c.i
.PHONY : src/tetris_game.i

# target to preprocess a source file
src/tetris_game.c.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tetris_offline.dir/build.make CMakeFiles/tetris_offline.dir/src/tetris_game.c.i
.PHONY : src/tetris_game.c.i

src/tetris_game.s: src/tetris_game.c.s
.PHONY : src/tetris_game.s

# target to generate assembly for a file
src/tetris_game.c.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/tetris_offline.dir/build.make CMakeFiles/tetris_offline.dir/src/tetris_game.c.s
.PHONY : src/tetris_game.c.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... server"
	@echo "... test_client_menu"
	@echo "... tetris_offline"
	@echo "... src/client/test_client_menu.o"
	@echo "... src/client/test_client_menu.i"
	@echo "... src/client/test_client_menu.s"
	@echo "... src/main.o"
	@echo "... src/main.i"
	@echo "... src/main.s"
	@echo "... src/server/server.o"
	@echo "... src/server/server.i"
	@echo "... src/server/server.s"
	@echo "... src/tetris_game.o"
	@echo "... src/tetris_game.i"
	@echo "... src/tetris_game.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

