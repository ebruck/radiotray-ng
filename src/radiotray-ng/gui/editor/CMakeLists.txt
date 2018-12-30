set(SRC_FILES
	editor_app.cpp
	editor_app.hpp
	editor_bookmarks.cpp
	editor_bookmarks.hpp
	editor_dialog_base.cpp
	editor_dialog_base.hpp
	editor_frame.cpp
	editor_frame.hpp
	group_drop_target.cpp
	group_drop_target.hpp
	group_editor_dialog.cpp
	group_editor_dialog.hpp
	group_list.cpp
	group_list.hpp
	image_list.cpp
	image_list.hpp
	station_drop_target.cpp
	station_drop_target.hpp
	station_editor_dialog.cpp
	station_editor_dialog.hpp
	station_list.cpp
	station_list.hpp
	status_dialog.cpp
	status_dialog.hpp
	wx_pch.hpp)

include(${wxWidgets_USE_FILE})
add_executable(rtng-bookmark-editor ${SRC_FILES})
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
