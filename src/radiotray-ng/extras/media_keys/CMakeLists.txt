if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    FILE(GLOB SRC_FILES "linux/*.cpp")
else()
    FILE(GLOB SRC_FILES "darwin/*.cpp")
endif()

add_library(media_keys STATIC ${SRC_FILES})

# todo: if appindicator add the includes
target_link_libraries(media_keys ${APPINDICATOR_GUI} ${GIOMM_LIBRARIES})
target_include_directories(media_keys SYSTEM PRIVATE ${GIOMM_INCLUDE_DIRS} ${APPINDICATOR_INCLUDE_DIRS} )
