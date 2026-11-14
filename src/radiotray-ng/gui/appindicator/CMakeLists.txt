# Native GTK3 menu and D-Bus StatusNotifierItem, without libayatana-appindicator.
add_library(appindicator_gui STATIC appindicator_gui.cpp appindicator_gui.hpp direct_sni.cpp direct_sni.hpp)
add_dependencies(appindicator_gui user-agent)
target_include_directories(appindicator_gui SYSTEM PRIVATE ${GTK3_INCLUDE_DIRS})
target_link_libraries(appindicator_gui PUBLIC ${GTK3_LIBRARIES})
