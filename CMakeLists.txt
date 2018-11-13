# davep 20181113 ; let's take a whack at CMake since CLion seems to like it 

cmake_minimum_required(VERSION 3.12)
project(wlan_netlink C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(wlan_netlink
        nl-wifi.c)
