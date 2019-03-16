message( "Building version: ${PROJECT_VERSION}..." )

set(BOOST_DEB_VERSION "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

set(CPACK_PACKAGE_VENDOR              "Edward G. Bruck")
set(CPACK_PACKAGE_CONTACT             "ed.bruck1@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple Internet Radio Player\n Radiotray-NG runs in the system tray allowing you to\n select and play configured radio stations.")
set(CPACK_PACKAGE_NAME                "radiotray-ng")
set(CPACK_PACKAGE_VERSION             ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR       ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR       ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH       ${PROJECT_VERSION_PATCH})
set(CPACK_DEBIAN_PACKAGE_SECTION      "sound")
set(CPACK_DEBIAN_COMPRESSION_TYPE     "xz")

if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(CPACK_SYSTEM_NAME                 "i386")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
else()
    set(CPACK_SYSTEM_NAME                 "amd64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
endif()

set(CPACK_DEBIAN_PACKAGE_PRIORITY     "optional")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER   "Edward G. Bruck <ed.bruck1@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS      "libc6, libbsd0, libcurl4, libjsoncpp1, libxdg-basedir1, libnotify4, python2.7, python-lxml, libglibmm-2.4-1v5, libboost-filesystem${BOOST_DEB_VERSION}, libboost-system${BOOST_DEB_VERSION}, libboost-log${BOOST_DEB_VERSION}, libboost-thread${BOOST_DEB_VERSION}, libboost-program-options${BOOST_DEB_VERSION}, libgstreamer1.0-0, libappindicator3-1, gstreamer1.0-plugins-good, gstreamer1.0-plugins-bad, gstreamer1.0-plugins-ugly, libwxgtk3.0-0v5")
set(CPACK_GENERATOR                   "DEB")
set(CPACK_PACKAGE_FILE_NAME            ${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${PROJECT_SOURCE_DIR}/debian/postinst;${PROJECT_SOURCE_DIR}/debian/postrm)

# no libcurl4 on Debian Stretch
execute_process(COMMAND lsb_release -c -s OUTPUT_VARIABLE CODENAME OUTPUT_STRIP_TRAILING_WHITESPACE)
# https://bugs.launchpad.net/ubuntu/+source/curl/+bug/1754294
execute_process(COMMAND lsb_release -r -s OUTPUT_VARIABLE RELEASE OUTPUT_STRIP_TRAILING_WHITESPACE)
if (${RELEASE} STREQUAL "16.04" OR ${CODENAME} STREQUAL "stretch")
   string(REPLACE "libcurl4" "libcurl3" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
endif()

include(CPack)

set(DESTINATION_DIR /usr/share/radiotray-ng)

execute_process(COMMAND gzip -n -9 -c "${PROJECT_SOURCE_DIR}/debian/changelog" WORKING_DIRECTORY ${PROJECT_BINARY_DIR} OUTPUT_FILE "${PROJECT_BINARY_DIR}/changelog.Debian.gz")

# for 16.04
execute_process(COMMAND chmod 755 ${PROJECT_SOURCE_DIR}/debian/postinst)
execute_process(COMMAND chmod 755 ${PROJECT_SOURCE_DIR}/debian/postrm)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/hicolor/256x256/apps/"              DESTINATION "/usr/share/icons/hicolor/256x256/apps"      FILES_MATCHING PATTERN "*.png")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/hicolor/24x24/apps/"                DESTINATION "/usr/share/icons/hicolor/24x24/apps"        FILES_MATCHING PATTERN "*.png")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/ubuntu-mono-dark/apps/24/"          DESTINATION "/usr/share/icons/ubuntu-mono-dark/apps/24"  FILES_MATCHING PATTERN "*.svg")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/data/ubuntu-mono-light/apps/24/"         DESTINATION "/usr/share/icons/ubuntu-mono-light/apps/24" FILES_MATCHING PATTERN "*.svg")
install(FILES     "${PROJECT_SOURCE_DIR}/data/about/radiotray-ng.png"             DESTINATION ${DESTINATION_DIR})
install(FILES     "${PROJECT_SOURCE_DIR}/data/bookmarks.json"                     DESTINATION ${DESTINATION_DIR})
install(PROGRAMS  "${PROJECT_SOURCE_DIR}/src/radiotray-ng/extras/scripts/rt2rtng" DESTINATION "/usr/bin/")
install(PROGRAMS  "${PROJECT_BINARY_DIR}/radiotray-ng"                            DESTINATION "/usr/bin/")
install(PROGRAMS  "${PROJECT_BINARY_DIR}/rtng-bookmark-editor"                    DESTINATION "/usr/bin/")
install(FILES     "${PROJECT_SOURCE_DIR}/data/radiotray-ng.desktop"               DESTINATION "/usr/share/applications/")
install(FILES     "${PROJECT_SOURCE_DIR}/data/rtng-bookmark-editor.desktop"       DESTINATION "/usr/share/applications/")
install(FILES     "${PROJECT_SOURCE_DIR}/data/radiotray-ng.appdata.xml"           DESTINATION "/usr/share/metainfo/")
install(FILES     "${PROJECT_SOURCE_DIR}/debian/copyright"                        DESTINATION "/usr/share/doc/${CPACK_PACKAGE_NAME}")
install(FILES     "${PROJECT_BINARY_DIR}/changelog.Debian.gz"                     DESTINATION "/usr/share/doc/${CPACK_PACKAGE_NAME}")
