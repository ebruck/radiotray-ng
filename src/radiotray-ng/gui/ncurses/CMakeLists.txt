FILE(GLOB SRC_FILES "*.cpp")

add_library(ncurses_gui STATIC ${SRC_FILES})
target_link_libraries(ncurses_gui)
