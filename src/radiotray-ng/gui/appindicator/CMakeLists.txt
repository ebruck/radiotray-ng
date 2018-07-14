FILE(GLOB SRC_FILES "*.cpp")

add_library(appindicator_gui STATIC ${SRC_FILES})
add_dependencies(appindicator_gui user-agent)
target_link_libraries(appindicator_gui)

target_compile_definitions(appindicator_gui PRIVATE APPINDICATOR_GUI)
target_include_directories(appindicator_gui SYSTEM PRIVATE ${APPINDICATOR_INCLUDE_DIRS})