FILE(GLOB SRC_FILES "*.cpp")

add_library(player STATIC ${SRC_FILES})
target_link_libraries(player ${APPINDICATOR_LIBRARIES})
add_dependencies(player user-agent)
target_include_directories(player PRIVATE ${GSTREAMER_INCLUDE_DIRS} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${APPINDICATOR_INCLUDE_DIRS})