# davep 20181113 ; let's take a whack at CMake since CLion seems to like it 

cmake_minimum_required(VERSION 3.11)
project(wlan_netlink C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

include_directories(.)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNL REQUIRED libnl-3.0 libnl-genl-3.0 libnl-route-3.0)

add_executable(wlan_netlink nl-wifi.c)
target_include_directories(wlan_netlink PUBLIC ${LIBNL_INCLUDE_DIRS})
target_link_libraries(wlan_netlink ${LIBNL_LIBRARIES})

add_executable(my-ip my-ip.c)
target_include_directories(my-ip PUBLIC ${LIBNL_INCLUDE_DIRS})
target_link_libraries(my-ip ${LIBNL_LIBRARIES})

