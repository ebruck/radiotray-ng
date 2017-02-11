if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    FILE(GLOB SRC_FILES "linux/*.cpp")
else()
    FILE(GLOB SRC_FILES "darwin/*.cpp")
endif()

add_library(notification STATIC ${SRC_FILES})
target_link_libraries(notification)

target_include_directories(notification PRIVATE ${NOTIFY_INCLUDE_DIRS})
