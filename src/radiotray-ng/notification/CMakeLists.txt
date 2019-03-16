if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(SRC_FILES linux/notification.cpp)
else()
    set(SRC_FILES darwin/notification.cpp)
endif()

add_library(notification STATIC ${SRC_FILES})
target_link_libraries(notification)

target_include_directories(notification PRIVATE ${NOTIFY_INCLUDE_DIRS})
