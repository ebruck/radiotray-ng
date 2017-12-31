FILE(GLOB SRC_FILES "*.cpp")
FILE(GLOB HDR_FILES "*.hpp")

include(${wxWidgets_USE_FILE})
add_executable(rtng-bookmark-editor ${SRC_FILES} ${HDR_FILES})
add_dependencies(rtng-bookmark-editor bookmarks config user-agent)

include_directories(rtng-bookmark-editor ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(rtng-bookmark-editor
	bookmarks
	config
	${Boost_LIBRARIES}
	${XDG_BASEDIR_LIBRARIES}
	${JSONCPP_LIBRARIES}
	${wxWidgets_LIBRARIES}
	${LIBBSD_LIBRARIES}
)

target_compile_definitions(rtng-bookmark-editor PRIVATE BOOKMARKS_EDITOR)
