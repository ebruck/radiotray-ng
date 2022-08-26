cmake_minimum_required(VERSION 3.10)

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
set(PROJECT_VERSION_PATCH 9-dev)
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

# set up wxWidgets to prefer gtk3
execute_process(COMMAND wx-config --selected-config --toolkit=gtk3
		RESULT_VARIABLE ret
		OUTPUT_QUIET)
if(ret EQUAL "0")
	message(STATUS "wxWidgets: Found gtk3 version, using it.")
	set(wxWidgets_CONFIG_OPTIONS_DEFAULT "--toolkit=gtk3")
else()
	message(STATUS "wxWidgets: No gtk3 version found, falling back to default (likely gtk2)")
endif()
unset(ret)
set(wxWidgets_CONFIG_OPTIONS "${wxWidgets_CONFIG_OPTIONS_DEFAULT}" ON STRING "wxWidgets toolkit options")
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
set(warnings "-Wall -Wextra -Werror -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${warnings}")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${warnings}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

# includes etc.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ ${JSONCPP_INCLUDE_DIRS})

# notification & ncurses based on platform
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    pkg_search_module(APPINDICATOR appindicator3-0.1)
    if (NOT APPINDICATOR_FOUND)
        pkg_search_module(APPINDICATOR REQUIRED ayatana-appindicator3-0.1)
        if (APPINDICATOR_FOUND)
            set(AYATANA_APPINDICATOR true)
        endif()
    endif()
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
        URL https://github.com/google/googletest/archive/release-1.8.1.tar.gz
        URL_HASH SHA256=9bf1fe5182a604b4135edc1a425ae356c9ad15e9b23f9f12a02e80184c3a249c
        TIMEOUT 30
        DOWNLOAD_NO_PROGRESS true
        INSTALL_COMMAND "")

    ExternalProject_Get_Property(googletest SOURCE_DIR)
    include_directories(${SOURCE_DIR}/googlemock/include ${SOURCE_DIR}/googletest/include)
    ExternalProject_Get_Property(googletest BINARY_DIR)
    link_directories(${BINARY_DIR}/googlemock ${BINARY_DIR}/googlemock/gtest)
    set(GMOCK_BOTH_LIBRARIES gmock_main gmock gtest)
    set_property(DIRECTORY PROPERTY CLEAN_NO_CUSTOM "${CMAKE_CURRENT_BINARY_DIR}/googletest")
    unset(SOURCE_DIR)
    unset(BINARY_DIR)

    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(tests/runners/)
endif()

set(BUILD_SHARED_LIBS NO)

add_subdirectory(package)
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
        -P ${PROJECT_SOURCE_DIR}/cmake/user-agent.cmake
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_custom_target(user-agent ALL DEPENDS rtng_user_agent)
