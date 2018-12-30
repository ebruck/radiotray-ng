if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(SRC_FILES linux/media_keys.cpp)
else()
    set(SRC_FILES darwin/media_keys.cpp)
endif()

add_library(media_keys STATIC ${SRC_FILES})

# todo: if appindicator add the includes
target_link_libraries(media_keys ${APPINDICATOR_GUI} ${GIOMM_LIBRARIES})
target_include_directories(media_keys SYSTEM PRIVATE ${GIOMM_INCLUDE_DIRS} ${APPINDICATOR_INCLUDE_DIRS} )
