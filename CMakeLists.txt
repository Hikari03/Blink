cmake_minimum_required(VERSION 4.1)
project(blink)

set(CMAKE_CXX_STANDARD 23)

# Include GTKMM
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-4.0)

link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

# Include libsodium
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSODIUM libsodium)

link_directories(${LIBSODIUM_LIBRARY_DIRS})
include_directories(${LIBSODIUM_INCLUDE_DIRS})

include_directories(server)
include_directories(src)

add_executable( blink-server
        server/accepter.cpp
        server/accepter.h
        server/cleaner.cpp
        server/cleaner.h
        server/Client.cpp
        server/Client.h
        server/ClientInfo.cpp
        server/ClientInfo.h
        server/main.cpp
        server/Message.cpp
        server/Message.h
        server/SharedResources.cpp
        server/SharedResources.h
        server/terminal.cpp
        server/terminal.h
)

add_executable( blink
        src/App.cpp
        src/App.h
        src/Connection.cpp
        src/Connection.h
        src/GTKHandler.cpp
        src/GTKHandler.h
        src/main.cpp
)