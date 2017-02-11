FILE(GLOB SRC_FILES "*.cpp")

add_library(event_bus STATIC ${SRC_FILES})
target_link_libraries(event_bus)
