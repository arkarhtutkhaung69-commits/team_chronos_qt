cmake_minimum_required(VERSION 3.16)

project(TEAM_CHRONOS_Qt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(TEAM_CHRONOS
    TEAM_CHRONOS_Qt.cpp
)

target_link_libraries(TEAM_CHRONOS PRIVATE Qt6::Widgets)

# Keep the text data files beside the executable when running from the build
# directory. The application also supports running from its project folder.
configure_file(users.txt users.txt COPYONLY)
configure_file(inventory.txt inventory.txt COPYONLY)
