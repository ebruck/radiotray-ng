FILE(GLOB SRC_FILES "*.cpp")

add_library(rtng_dbus STATIC ${SRC_FILES})

target_link_libraries(rtng_dbus ${GLIBMM_LIBRARIES} ${GIOMM_LIBRARIES})
target_include_directories(rtng_dbus SYSTEM PRIVATE ${GIOMM_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
