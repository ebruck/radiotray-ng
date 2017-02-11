FILE(GLOB SRC_FILES "*.cpp")

add_library(config STATIC ${SRC_FILES})
target_link_libraries(config )
