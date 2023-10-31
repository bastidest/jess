cmake_minimum_required(VERSION 3.27)
project(jess)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
add_library(ncurses INTERFACE)
target_link_libraries(ncurses INTERFACE ${CURSES_LIBRARIES})
target_compile_options(ncurses INTERFACE ${CURSES_CFLAGS})
target_include_directories(ncurses INTERFACE ${CURSES_INCLUDE_DIRS})

find_package(Systemd REQUIRED)
add_library(systemd INTERFACE)
target_link_libraries(systemd INTERFACE ${SYSTEMD_LIBRARIES})
target_compile_options(systemd INTERFACE ${SYSTEMD_CFLAGS})
target_include_directories(systemd INTERFACE ${SYSTEMD_INCLUDE_DIRS})

add_executable(jess src/main.cpp
        src/NcWindow.hpp
        src/NcError.hpp
        src/NcTerminal.hpp)
target_link_libraries(jess ncurses systemd)
