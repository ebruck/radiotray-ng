find_package (Gettext QUIET)

if (GETTEXT_FOUND)
    find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
    if(NOT GETTEXT_XGETTEXT_EXECUTABLE OR NOT GETTEXT_MSGFMT_EXECUTABLE)
        message(FATAL_ERROR "xgettext not found")
    endif()

    find_program(GETTEXT_MSGINIT_EXECUTABLE msginit)
    if(NOT GETTEXT_MSGINIT_EXECUTABLE)
        message(FATAL_ERROR "msginit not found")
    endif()

    find_program(GETTEXT_MSGCAT_EXECUTABLE msgcat)
    if(NOT GETTEXT_MSGCAT_EXECUTABLE )
        message("msgcat not found")
        set(TRANSLATION_TOOLS_FOUND false)
    endif(NOT GETTEXT_MSGCAT_EXECUTABLE )

    find_program(GETTEXT_MSGATTRIB_EXECUTABLE msgattrib)
    if(NOT GETTEXT_MSGATTRIB_EXECUTABLE)
        message("msgattrib not found")
        set(TRANSLATION_TOOLS_FOUND false)
    endif(NOT GETTEXT_MSGATTRIB_EXECUTABLE)

    find_program(GETTEXT_MSGCAT_EXECUTABLE msgcat)
    if(NOT GETTEXT_MSGCAT_EXECUTABLE)
        message("msgcat not found")
        set(TRANSLATION_TOOLS_FOUND false)
    endif(NOT GETTEXT_MSGCAT_EXECUTABLE)

    option (UPDATE_PO "Update po files" OFF)
    option (CREATE_MO "Create mo files" ON)

    option (LANGUAGES "Using translations")

    ##
    # Supported Languages
    list (APPEND AllLanguages
            en
            ru
            )
    if (NOT DEFINED linguas)
        set (LANGUAGES ${AllLanguages} CACHE STRING "Using translations" FORCE)
    else (NOT DEFINED linguas)
        if (NOT linguas)
            set (LANGUAGES "" CACHE STRING "Using translations" FORCE)
        elseif (linguas STREQUAL *)
            set (LANGUAGES ${AllLanguages} CACHE STRING "Using translations" FORCE)
        else (NOT linguas)
            string(REGEX MATCHALL [a-zA-Z_@]+
                    linguas1 ${linguas})
            set (LANGUAGES ${linguas1} CACHE STRING "Using translations" FORCE)
        endif (NOT linguas)
    endif (NOT DEFINED linguas)
    message (STATUS "Translations: ${LANGUAGES}")


    ##
    # All "po" dirs in all projects
    list(APPEND i18n_po
            "src/radiotray-ng/gui/appindicator/po"
            )

    ##
    # Map
    #         key - .mo filename;
    #       value - "po" dirs (separated by ' + ')  - sources for .mo file.
    #
    # example: "some_GettextDomain.mo => src/some/dir/po + src/someOther/dir/po"
    list(APPEND i18n_mo
            "radiotray-ng.mo => src/radiotray-ng/gui/appindicator/po"
            )

    include("UpdatePO.cmake")
    include("GenerateMO.cmake")


    ##
    # Build deps & defines
    if(CREATE_MO)
        add_dependencies(radiotray-ng mo-update)
        add_dependencies(rtng-bookmark-editor mo-update)
    endif(CREATE_MO)

    set(HAVE_GETTEXT 1)
    set(GETTEXT_DOMAIN "radiotray-ng")
endif (GETTEXT_FOUND)

configure_file(${PROJECT_SOURCE_DIR}/cmake/i18n/i18n.hpp.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/i18n.hpp.tmp)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/i18n.hpp.tmp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rtng_i18n.hpp)
execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/i18n.hpp.tmp)
