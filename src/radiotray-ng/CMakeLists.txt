add_executable(radiotray-ng main.cpp radiotray_ng.cpp)

target_link_libraries(
  radiotray-ng  
  bookmarks  
  config
  event_bus
  player
  playlist
  ncurses_gui
  appindicator_gui
  notification
  media_keys
  ${NOTIFY_LIBRARIES}
  ${NCURSES_LIBRARIES}
  ${CURL_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${Boost_LIBRARIES}
  ${XDG_BASEDIR_LIBRARIES}
  ${JSONCPP_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${GIO_LIBRARIES}
  ${APPINDICATOR_LIBRARIES}    
  pthread
)
  
add_dependencies(radiotray-ng user-agent)
target_include_directories(radiotray-ng PRIVATE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${GSTREAMER_INCLUDE_DIRS} ${APPINDICATOR_INCLUDE_DIRS})
