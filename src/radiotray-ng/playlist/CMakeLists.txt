FILE(GLOB SRC_FILES "*.cpp")

add_library(playlist STATIC ${SRC_FILES})
target_link_libraries(playlist)
add_dependencies(playlist user-agent)
target_include_directories(playlist PRIVATE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})