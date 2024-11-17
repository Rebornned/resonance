cmake_minimum_required(VERSION 3.29)
project(Final_Project_Prog2_Playlists C)

set(CMAKE_C_STANDARD 11)

include_directories(bin)

add_executable(Final_Project_Prog2_Playlists
        bin/func.c
        bin/main.c
)
