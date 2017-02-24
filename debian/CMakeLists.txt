message( "Building version: ${PROJECT_VERSION}..." )

# only tested on 16.04 & 16.10

set(CPACK_PACKAGE_VENDOR              "Edward G. Bruck")
set(CPACK_PACKAGE_CONTACT             "ed.bruck1@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple Internet Radio Player for Ubuntu\n Radiotray-NG runs in the system tray allowing you to\n select and play configured radio stations.")
set(CPACK_PACKAGE_NAME                "radiotray-ng")
set(CPACK_PACKAGE_VERSION             ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR       ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR       ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH       ${PROJECT_VERSION_PATCH})
set(CPACK_DEBIAN_PACKAGE_SECTION      "sound")

if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(CPACK_SYSTEM_NAME                 "i386")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
else()
    set(CPACK_SYSTEM_NAME                 "amd64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
endif()

set(CPACK_DEBIAN_PACKAGE_PRIORITY     "optional")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER   "Edward G. Bruck <ed.bruck1@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS      "libc6, libbsd0, libcurl3, libjsoncpp1, libxdg-basedir1, libnotify4, libboost-filesystem1.58.0 | libboost-filesystem1.61.0, libboost-system1.58.0 | libboost-system1.61.0, libboost-log1.58.0 | libboost-log1.61.0, libboost-thread1.58.0 | libboost-thread1.61.0, libgstreamer1.0-0, libappindicator1, gstreamer1.0-plugins-good, gstreamer1.0-plugins-bad")
set(CPACK_GENERATOR                   "DEB")
set(CPACK_PACKAGE_FILE_NAME            ${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${PROJECT_SOURCE_DIR}/debian/postinst;${PROJECT_SOURCE_DIR}/debian/postrm)

INCLUDE(CPack)
 
set(DESTINATION_DIR /usr/share/radiotray-ng)

execute_process(COMMAND gzip -9 -c "${PROJECT_SOURCE_DIR}/debian/changelog" WORKING_DIRECTORY ${PROJECT_BINARY_DIR} OUTPUT_FILE "${PROJECT_BINARY_DIR}/changelog.gz")

# for 16.04
execute_process(COMMAND chmod 755 ${PROJECT_SOURCE_DIR}/debian/postinst)
execute_process(COMMAND chmod 755 ${PROJECT_SOURCE_DIR}/debian/postrm)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/hicolor/256x256/apps/"      DESTINATION "/usr/share/icons/hicolor/256x256/apps"      FILES_MATCHING PATTERN "*.png")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/hicolor/24x24/apps/"        DESTINATION "/usr/share/icons/hicolor/24x24/apps"        FILES_MATCHING PATTERN "*.png")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/ubuntu-mono-dark/apps/24/"  DESTINATION "/usr/share/icons/ubuntu-mono-dark/apps/24"  FILES_MATCHING PATTERN "*.svg")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/ubuntu-mono-light/apps/24/" DESTINATION "/usr/share/icons/ubuntu-mono-light/apps/24" FILES_MATCHING PATTERN "*.svg")
install(FILES     "${PROJECT_SOURCE_DIR}/data/about/radiotray-ng.png"     DESTINATION ${DESTINATION_DIR})
install(FILES     "${PROJECT_SOURCE_DIR}/data/bookmarks.json"             DESTINATION ${DESTINATION_DIR})
install(FILES     "${PROJECT_SOURCE_DIR}/COPYING"                         DESTINATION ${DESTINATION_DIR})
install(PROGRAMS  "${PROJECT_BINARY_DIR}/radiotray-ng"                    DESTINATION "/usr/bin/")
install(FILES     "${PROJECT_SOURCE_DIR}/data/radiotray-ng.desktop"       DESTINATION "/usr/share/applications/")
install(FILES     "${PROJECT_SOURCE_DIR}/debian/copyright"                DESTINATION "/usr/share/doc/${CPACK_PACKAGE_NAME}")
install(FILES     "${PROJECT_BINARY_DIR}/changelog.gz"                    DESTINATION "/usr/share/doc/${CPACK_PACKAGE_NAME}")
