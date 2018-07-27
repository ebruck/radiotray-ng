cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(radiotray-ng CXX)

# workaround for Eclipse
if (${CMAKE_EXTRA_GENERATOR} MATCHES "Eclipse CDT4")
	set(CMAKE_CXX_COMPILER_ARG1 "-std=c++14"
		CACHE STRING "C++ version for Eclipse's indexer" FORCE)
	set(CMAKE_ECLIPSE_VERSION "4.4"
		CACHE STRING "Eclipse version" FORCE)
endif()

# version for user agent creation
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 2)
set(PROJECT_VERSION_PATCH 4-dev)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# output dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# use ccache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK    ccache)
endif(CCACHE_FOUND)

# search for libs
find_package(CURL      REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Boost     REQUIRED filesystem system log program_options thread)
find_package(wxWidgets REQUIRED core base adv)

pkg_search_module(JSONCPP     REQUIRED jsoncpp)
pkg_search_module(GSTREAMER   REQUIRED gstreamer-1.0)
pkg_search_module(XDG_BASEDIR REQUIRED libxdg-basedir)
if (NOT APPLE)
    pkg_search_module(LIBBSD  REQUIRED libbsd)
endif(NOT APPLE)

# flags
set(CMAKE_CXX_STANDARD 14)
add_definitions("-DBOOST_LOG_DYN_LINK")
add_compile_options("-fdiagnostics-color=auto")
set(warnings "-Wno-deprecated-declarations -Wall -Wextra -Werror -Wpedantic")
set(CMAKE_CXX_FLAGS ${warnings})
set(CMAKE_C_FLAGS   ${warnings})
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

# includes etc.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ ${JSONCPP_INCLUDE_DIRS})

# notification & ncurses based on platform
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    pkg_search_module(APPINDICATOR REQUIRED appindicator3-0.1)
if (NOT APPINDICATOR_FOUND)
    pkg_search_module(NCURSES      REQUIRED ncurses)
endif()
    pkg_search_module(NOTIFY       REQUIRED libnotify)
    pkg_search_module(GLIBMM       REQUIRED glibmm-2.4)
    pkg_search_module(GIOMM        REQUIRED giomm-2.4)
else()
    set(NCURSES_FOUND true)
    set(NCURSES_LIBRARIES ncurses)
    set(NOTIFY_FOUND true)
    set(APPINDICATOR_FOUND false)
endif()

# build tests? Then we need googletest...
if (BUILD_TESTS)
    include(ExternalProject)

    ExternalProject_Add(googletest
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}/googletest"
        GIT_REPOSITORY https://github.com/google/googletest.git
        INSTALL_COMMAND ""
        GIT_TAG release-1.8.0
        GIT_SHALLOW true)

    ExternalProject_Get_Property(googletest SOURCE_DIR)
    include_directories(${SOURCE_DIR}/googlemock/include ${SOURCE_DIR}/googletest/include)
    ExternalProject_Get_Property(googletest BINARY_DIR)
    link_directories(${BINARY_DIR}/googlemock)
    set(GMOCK_BOTH_LIBRARIES gmock_main gmock)
    unset(SOURCE_DIR)
    unset(BINARY_DIR)

    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(tests/runners/)
endif()

add_subdirectory(debian)
add_subdirectory(src/radiotray-ng)
add_subdirectory(src/radiotray-ng/config)
add_subdirectory(src/radiotray-ng/playlist)
add_subdirectory(src/radiotray-ng/bookmarks)
add_subdirectory(src/radiotray-ng/event_bus)
add_subdirectory(src/radiotray-ng/player)
add_subdirectory(src/radiotray-ng/notification)
add_subdirectory(src/radiotray-ng/extras/media_keys)
add_subdirectory(src/radiotray-ng/gui/editor)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_subdirectory(src/radiotray-ng/extras/rtng_dbus)
    target_compile_definitions(radiotray-ng PRIVATE RTNG_DBUS)
endif()

# which gui?
if (APPINDICATOR_FOUND)
    target_compile_definitions(radiotray-ng PRIVATE APPINDICATOR_GUI)
    add_subdirectory(src/radiotray-ng/gui/appindicator)
else()
    add_subdirectory(src/radiotray-ng/gui/ncurses)
endif()

# user agent
add_custom_command(
     OUTPUT  rtng_user_agent
    COMMENT "Generating rtng_user_agent.hpp"
    COMMAND ${CMAKE_COMMAND}
        -D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -D CMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        -D CMAKE_PROJECT_NAME=${CMAKE_PROJECT_NAME}
        -D PROJECT_VERSION=${PROJECT_VERSION}
        -D CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
        -D CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}
        -P ${PROJECT_SOURCE_DIR}/user-agent.cmake
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_custom_target(user-agent ALL DEPENDS rtng_user_agent)
