add_library(mpris_dbus STATIC mpris_dbus.cpp mpris_dbus.hpp)
target_link_libraries(mpris_dbus ${GLIBMM_LIBRARIES} ${GIOMM_LIBRARIES})
target_include_directories(mpris_dbus SYSTEM PRIVATE ${GIOMM_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
