# test "runner" apps
if (BUILD_TESTS)
    add_executable(playlist_runner playlist_runner.cpp)
    target_link_libraries(playlist_runner playlist config ${JSONCPP_LIBRARIES} ${XDG_BASEDIR_LIBRARIES} ${Boost_LIBRARIES} ${CURL_LIBRARIES} pthread)
    target_include_directories(playlist_runner PRIVATE ${JSONCPP_INCLUDE_DIRS})

    add_executable(bookmarks_runner bookmarks_runner.cpp)
    target_link_libraries(bookmarks_runner bookmarks ${JSONCPP_LIBRARIES} ${Boost_LIBRARIES} pthread)
    target_include_directories(bookmarks_runner PRIVATE ${JSONCPP_INCLUDE_DIRS})

    add_executable(notification_runner notification_runner.cpp)
    target_link_libraries(notification_runner notification ${NOTIFY_LIBRARIES} ${Boost_LIBRARIES} pthread)
    target_include_directories(notification_runner PRIVATE ${NOTIFY_INCLUDE_DIRS})

    add_executable(player_runner player_runner.cpp)
    target_link_libraries(player_runner player event_bus notification config ${JSONCPP_LIBRARIES} ${NOTIFY_LIBRARIES} ${Boost_LIBRARIES} ${GSTREAMER_LIBRARIES} pthread)
    target_include_directories(player_runner PRIVATE ${GSTREAMER_INCLUDE_DIRS} ${JSONCPP_INCLUDE_DIRS})
endif()
