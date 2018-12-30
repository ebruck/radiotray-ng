add_library(rtng_dbus STATIC rtng_dbus.cpp rtng_dbus.hpp)
target_link_libraries(rtng_dbus ${GLIBMM_LIBRARIES} ${GIOMM_LIBRARIES})
target_include_directories(rtng_dbus SYSTEM PRIVATE ${GIOMM_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
