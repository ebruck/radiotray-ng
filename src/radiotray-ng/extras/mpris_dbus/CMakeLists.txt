FILE(GLOB SRC_FILES "*.cpp")

add_library(mpris_dbus STATIC ${SRC_FILES})

target_link_libraries(mpris_dbus ${GLIBMM_LIBRARIES} ${GIOMM_LIBRARIES})
target_include_directories(mpris_dbus SYSTEM PRIVATE ${GIOMM_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
