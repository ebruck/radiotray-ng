# gather system info...
find_program(LSB_RELEASE_EXECUTABLE NAMES lsb_release)

if (LSB_RELEASE_EXECUTABLE)
    execute_process(COMMAND lsb_release -c -s OUTPUT_VARIABLE CODENAME        OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND lsb_release -i -s OUTPUT_VARIABLE DISTRIBUTOR_ID  OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND lsb_release -r -s OUTPUT_VARIABLE RELEASE         OUTPUT_STRIP_TRAILING_WHITESPACE)

    # default to debian...
    string(TOLOWER ${DISTRIBUTOR_ID} DISTRIBUTOR_ID)
    if (NOT CPACK_GENERATOR)
        if (DISTRIBUTOR_ID STREQUAL "fedora")
            set(CPACK_GENERATOR "RPM")
        else()
            set(CPACK_GENERATOR "DEB")
        endif()
    endif()

    message( "Building version: ${PROJECT_VERSION} for ${DISTRIBUTOR_ID} ${RELEASE} -- ${CPACK_GENERATOR} packaging..." )

    set(CPACK_PACKAGE_VENDOR              "Edward G. Bruck")
    set(CPACK_PACKAGE_CONTACT             "ed.bruck1@gmail.com")
    set(CPACK_PACKAGE_NAME                "radiotray-ng")
    set(CPACK_PACKAGE_VERSION             ${PROJECT_VERSION})
    set(CPACK_PACKAGE_VERSION_MAJOR       ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR       ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH       ${PROJECT_VERSION_PATCH})
    set(CPACK_PACKAGE_RELOCATABLE         FALSE)
    set(DESTINATION_DIR                   /usr/share/radiotray-ng/)

    if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
        set(CPACK_SYSTEM_NAME "i386")
    else()
        if (DISTRIBUTOR_ID STREQUAL "fedora")
            set(CPACK_SYSTEM_NAME "x86_64")
        else()
            set(CPACK_SYSTEM_NAME "amd64")
        endif()
    endif()

    set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}_${DISTRIBUTOR_ID}_${RELEASE}_${CPACK_SYSTEM_NAME})

    if (CPACK_GENERATOR MATCHES "DEB")
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY  "Simple Internet Radio Player\n Radiotray-NG runs in the system tray allowing you to\n select and play configured radio stations.")
        set(BOOST_DEB_VERSION                  "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
        set(CPACK_DEBIAN_PACKAGE_SECTION       "sound")
        set(CPACK_DEBIAN_COMPRESSION_TYPE      "xz")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE  ${CPACK_SYSTEM_NAME})
        set(CPACK_DEBIAN_PACKAGE_PRIORITY      "optional")
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER    "Edward G. Bruck <ed.bruck1@gmail.com>")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS       "libc6, libbsd0, libcurl4, libjsoncpp1, libxdg-basedir1, libnotify4, python3-lxml, libglibmm-2.4-1v5, libboost-filesystem${BOOST_DEB_VERSION}, libboost-system${BOOST_DEB_VERSION}, libboost-log${BOOST_DEB_VERSION}, libboost-thread${BOOST_DEB_VERSION}, libboost-program-options${BOOST_DEB_VERSION}, libgstreamer1.0-0, libappindicator3-1, gstreamer1.0-plugins-good, gstreamer1.0-plugins-bad, gstreamer1.0-plugins-ugly, libwxgtk3.0-gtk3-0v5")
        set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${PROJECT_SOURCE_DIR}/package/postinst;${PROJECT_SOURCE_DIR}/package/postrm;${PROJECT_SOURCE_DIR}/package/conffiles)

        if (${RELEASE} STREQUAL "21.04")
            string(REPLACE "libjsoncpp1" "libjsoncpp24" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
        endif()

        if (${RELEASE} STREQUAL "22.04" OR ${RELEASE} STREQUAL "23.04" OR ${RELEASE} STREQUAL "23.10")
            string(REPLACE "libjsoncpp1" "libjsoncpp25" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
            string(REPLACE "libappindicator3-1" "libayatana-appindicator3-1" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
        endif()

        # no libcurl4 on Debian Stretch
        # https://bugs.launchpad.net/ubuntu/+source/curl/+bug/1754294
        if (${RELEASE} STREQUAL "16.04" OR ${CODENAME} STREQUAL "stretch")
            string(REPLACE "libcurl4" "libcurl3" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
        endif()

        # Debian 12
        if (${CODENAME} STREQUAL "bookworm")
            string(REPLACE "libjsoncpp1" "libjsoncpp25" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
            string(REPLACE "libappindicator3-1" "libayatana-appindicator3-1" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
            string(REPLACE "libwxgtk3.0-gtk3-0v5" "libwxgtk3.2-1" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
        endif()       

        # Debian no longer supports libjsoncpp1 and ubuntu appindicator package naming
        if (${CODENAME} STREQUAL "bullseye")
            string(REPLACE "libjsoncpp1" "libjsoncpp24" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
            string(REPLACE "libappindicator3-1" "libayatana-appindicator3-1" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
        endif()

        # official packages for older distros will continue to use gtk2 for the editor
        if (${RELEASE} STREQUAL "16.04" OR ${RELEASE} STREQUAL "18.04" OR ${RELEASE} STREQUAL "19.04" OR ${CODENAME} STREQUAL "stretch")
            message(STATUS "Official packaging assumes gtk2 dependency for 'rtng-bookmark-editor'")
            string(REPLACE "libwxgtk3.0-gtk3-0v5" "libwxgtk3.0-0v5" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
        endif()

        # ubuntu lunar has newer libwsxgtk
        if (${RELEASE} STREQUAL "23.04" OR ${RELEASE} STREQUAL "23.10")
            message(STATUS "Official packaging assumes gtk2 dependency for 'rtng-bookmark-editor'")
            string(REPLACE "libwxgtk3.0-gtk3-0v5" "libwxgtk3.2-1" CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS})
        endif()

        execute_process(COMMAND gzip -n -9 -c "${PROJECT_SOURCE_DIR}/package/changelog-deb" WORKING_DIRECTORY ${PROJECT_BINARY_DIR} OUTPUT_FILE "${PROJECT_BINARY_DIR}/changelog.Debian.gz")

        # for 16.04
        execute_process(COMMAND chmod 755 ${PROJECT_SOURCE_DIR}/package/postinst)
        execute_process(COMMAND chmod 755 ${PROJECT_SOURCE_DIR}/package/postrm)
        execute_process(COMMAND chmod 644 ${PROJECT_SOURCE_DIR}/package/conffiles)

        install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/themes/ubuntu-mono-dark/apps/24/  DESTINATION /usr/share/icons/ubuntu-mono-dark/apps/24  FILES_MATCHING PATTERN *.svg)
        install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/themes/ubuntu-mono-light/apps/24/ DESTINATION /usr/share/icons/ubuntu-mono-light/apps/24 FILES_MATCHING PATTERN *.svg)
        install(FILES     ${PROJECT_SOURCE_DIR}/package/copyright                      DESTINATION /usr/share/doc/${CPACK_PACKAGE_NAME})
        install(FILES     ${PROJECT_BINARY_DIR}/changelog.Debian.gz                    DESTINATION /usr/share/doc/${CPACK_PACKAGE_NAME})
    endif()

    if (CPACK_GENERATOR MATCHES "RPM")
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY  "Simple Internet Radio Player. Radiotray-NG runs in the system tray allowing you to select and play configured radio stations.")
        set(CPACK_RPM_PACKAGE_URL      "https://www.github.com/ebruck/radiotray-ng")
        set(CPACK_RPM_PACKAGE_REQUIRES "glibc, libcurl, jsoncpp, boost, libxdg-basedir, libbsd, libappindicator-gtk3, libnotify, glibmm24, python3-lxml, gstreamer-plugins-base, gstreamer-plugins-good, gstreamer-plugins-bad-free, wxGTK3")
        set(CPACK_RPM_PACKAGE_GROUP    "Applications/Multimedia")
        set(CPACK_RPM_PACKAGE_LICENSE  "GPLv3")
        set(CPACK_RPM_COMPRESSION_TYPE "xz")
        set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE   ${PROJECT_SOURCE_DIR}/package/postinst)
        set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/package/postrm)

        install(FILES ${PROJECT_SOURCE_DIR}/README.md DESTINATION /usr/share/doc/${CPACK_PACKAGE_NAME})
        install(FILES ${PROJECT_SOURCE_DIR}/AUTHORS   DESTINATION /usr/share/doc/${CPACK_PACKAGE_NAME})
        install(FILES ${PROJECT_SOURCE_DIR}/COPYING   DESTINATION /usr/share/licences/${CPACK_PACKAGE_NAME})

        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/applications)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/metainfo)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/licences)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/hicolor)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/hicolor/24x24)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/hicolor/24x24/apps)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/hicolor/256x256)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/hicolor/256x256/apps)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/breeze)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/breeze/apps)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/breeze/apps/22)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/breeze/apps/48)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/Yaru)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/Yaru/256x256)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons/Yaru/256x256/apps)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /etc/xdg)
        list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /etc/xdg/autostart)

        set(CPACK_RPM_CHANGELOG_FILE ${PROJECT_SOURCE_DIR}/package/changelog-rpm)
    endif()

    install(FILES ${PROJECT_SOURCE_DIR}/data/themes/hicolor/256x256/apps/radiotray-ng-on.png
                  ${PROJECT_SOURCE_DIR}/data/themes/hicolor/256x256/apps/radiotray-ng-off.png
                  ${PROJECT_SOURCE_DIR}/data/themes/hicolor/256x256/apps/radiotray-ng-notification.png
                  DESTINATION /usr/share/icons/hicolor/256x256/apps)

    install(FILES ${PROJECT_SOURCE_DIR}/data/themes/hicolor/24x24/apps/radiotray-ng-on.png
                  ${PROJECT_SOURCE_DIR}/data/themes/hicolor/24x24/apps/radiotray-ng-off.png
                  DESTINATION /usr/share/icons/hicolor/24x24/apps)

    install(FILES ${PROJECT_SOURCE_DIR}/data/themes/Yaru/256x256/apps/radiotray-ng-notification.png
                  DESTINATION /usr/share/icons/Yaru/256x256/apps)

    install(FILES ${PROJECT_SOURCE_DIR}/data/themes/breeze/apps/22/radiotray-ng-on.svg
                  ${PROJECT_SOURCE_DIR}/data/themes/breeze/apps/22/radiotray-ng-off.svg
                  DESTINATION /usr/share/icons/breeze/apps/22)

    install(FILES ${PROJECT_SOURCE_DIR}/data/themes/breeze/apps/48/radiotray-ng-notification.svg
                  DESTINATION /usr/share/icons/breeze/apps/48)

    install(FILES     ${PROJECT_SOURCE_DIR}/data/about/radiotray-ng.png             DESTINATION ${DESTINATION_DIR})
    install(FILES     ${PROJECT_SOURCE_DIR}/data/bookmarks.json                     DESTINATION ${DESTINATION_DIR})
    install(FILES     ${PROJECT_SOURCE_DIR}/data/radiotray-ng.desktop               DESTINATION /usr/share/applications/)
    install(FILES     ${PROJECT_SOURCE_DIR}/data/rtng-bookmark-editor.desktop       DESTINATION /usr/share/applications/)
    install(FILES     ${PROJECT_SOURCE_DIR}/data/radiotray-ng.appdata.xml           DESTINATION /usr/share/metainfo/)
    install(FILES     ${PROJECT_SOURCE_DIR}/data/radiotray-ng.desktop               DESTINATION /etc/xdg/autostart/)
    install(PROGRAMS  ${PROJECT_SOURCE_DIR}/src/radiotray-ng/extras/scripts/rt2rtng DESTINATION /usr/bin/)
    install(PROGRAMS  ${PROJECT_BINARY_DIR}/radiotray-ng                            DESTINATION /usr/bin/)
    install(PROGRAMS  ${PROJECT_BINARY_DIR}/rtng-bookmark-editor                    DESTINATION /usr/bin/)

    include(CPack)
else()
    message(AUTHOR_WARNING "lsb_release not found! -- packaging disabled")
endif()
