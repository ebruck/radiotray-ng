add_executable(radiotray-ng main.cpp radiotray_ng.cpp radiotray_ng.hpp)

if (APPINDICATOR_FOUND)
    if (AYATANA_APPINDICATOR)
        target_compile_definitions(radiotray-ng PRIVATE AYATANA_APPINDICATOR)
    endif()

    target_link_libraries(
        radiotray-ng
        appindicator_gui
        ${APPINDICATOR_LIBRARIES})
else()
    target_link_libraries(
        radiotray-ng
        ncurses_gui
        ${NCURSES_LIBRARIES})
endif()    

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(radiotray-ng rtng_dbus)
endif()

target_link_libraries(
  radiotray-ng  
  bookmarks  
  config
  event_bus
  player
  playlist
  notification
  media_keys
  ${NOTIFY_LIBRARIES}
  ${CURL_LIBRARIES}
  ${Boost_LIBRARIES}
  ${XDG_BASEDIR_LIBRARIES}
  ${JSONCPP_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${GIO_LIBRARIES}
  ${LIBBSD_LIBRARIES}
  pthread
)

add_dependencies(radiotray-ng user-agent)
target_include_directories(radiotray-ng SYSTEM PRIVATE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${GSTREAMER_INCLUDE_DIRS} ${APPINDICATOR_INCLUDE_DIRS} ${GIOMM_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
