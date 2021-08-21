add_library(appindicator_gui STATIC appindicator_gui.cpp appindicator_gui.hpp)
add_dependencies(appindicator_gui user-agent)

if (AYATANA_APPINDICATOR)
    target_compile_definitions(appindicator_gui PRIVATE APPINDICATOR_GUI AYATANA_APPINDICATOR)
else()
    target_compile_definitions(appindicator_gui PRIVATE APPINDICATOR_GUI)
endif()
target_include_directories(appindicator_gui SYSTEM PRIVATE ${APPINDICATOR_INCLUDE_DIRS})
