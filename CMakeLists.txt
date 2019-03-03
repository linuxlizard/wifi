# davep 20181113 ; let's take a whack at CMake since CLion seems to like it 

cmake_minimum_required(VERSION 3.11)
project(wlan_netlink VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

set(MY_DEBUG_OPTIONS -Wall -Wextra -Wshadow -pedantic)

include_directories(.)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNL REQUIRED libnl-3.0 libnl-genl-3.0 libnl-route-3.0)

add_library(hdump OBJECT hdump.c)

add_executable(wlan_netlink $<TARGET_OBJECTS:hdump> nl-wifi.c)
target_include_directories(wlan_netlink PUBLIC ${LIBNL_INCLUDE_DIRS})
target_link_libraries(wlan_netlink ${LIBNL_LIBRARIES})
target_compile_options(wlan_netlink PUBLIC $<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>)

add_executable(my-ip my-ip.c)
target_include_directories(my-ip PUBLIC ${LIBNL_INCLUDE_DIRS})
target_link_libraries(my-ip ${LIBNL_LIBRARIES})

#https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
#-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic
set(CPP_DEBUG_OPTIONS -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic)

set(MAKE_CXX_STANDARD 17)
add_executable(scan scan.cc)
set_target_properties(scan PROPERTIES CXX_STANDARD 17)
target_compile_features(scan PUBLIC cxx_std_17)
target_compile_options(scan PUBLIC $<$<CONFIG:DEBUG>:${CPP_DEBUG_OPTIONS}>)
