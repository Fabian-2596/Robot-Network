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
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.28.0/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.28.0/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/build"

# Include any dependencies generated for this target.
include CMakeFiles/testP1.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/testP1.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/testP1.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testP1.dir/flags.make

CMakeFiles/testP1.dir/test/testP1.cpp.o: CMakeFiles/testP1.dir/flags.make
CMakeFiles/testP1.dir/test/testP1.cpp.o: /Users/fabianschmidt/Library/Mobile\ Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/test/testP1.cpp
CMakeFiles/testP1.dir/test/testP1.cpp.o: CMakeFiles/testP1.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir="/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/testP1.dir/test/testP1.cpp.o"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testP1.dir/test/testP1.cpp.o -MF CMakeFiles/testP1.dir/test/testP1.cpp.o.d -o CMakeFiles/testP1.dir/test/testP1.cpp.o -c "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/test/testP1.cpp"

CMakeFiles/testP1.dir/test/testP1.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/testP1.dir/test/testP1.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/test/testP1.cpp" > CMakeFiles/testP1.dir/test/testP1.cpp.i

CMakeFiles/testP1.dir/test/testP1.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/testP1.dir/test/testP1.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/test/testP1.cpp" -o CMakeFiles/testP1.dir/test/testP1.cpp.s

# Object files for target testP1
testP1_OBJECTS = \
"CMakeFiles/testP1.dir/test/testP1.cpp.o"

# External object files for target testP1
testP1_EXTERNAL_OBJECTS =

testP1: CMakeFiles/testP1.dir/test/testP1.cpp.o
testP1: CMakeFiles/testP1.dir/build.make
testP1: CMakeFiles/testP1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir="/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/build/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable testP1"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testP1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testP1.dir/build: testP1
.PHONY : CMakeFiles/testP1.dir/build

CMakeFiles/testP1.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testP1.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testP1.dir/clean

CMakeFiles/testP1.dir/depend:
	cd "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/build" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt" "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt" "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/build" "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/build" "/Users/fabianschmidt/Library/Mobile Documents/com~apple~CloudDocs/Uni/Sem7/VS/git/Do4x-Team-D-Gutberlet_Schmidt/build/CMakeFiles/testP1.dir/DependInfo.cmake" "--color=$(COLOR)"
.PHONY : CMakeFiles/testP1.dir/depend

