find_program(LSB_RELEASE_EXECUTABLE NAMES lsb_release)

if (LSB_RELEASE_EXECUTABLE)
    execute_process(COMMAND lsb_release -i -s OUTPUT_VARIABLE USER_AGENT_DISTRO          OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND lsb_release -c -s OUTPUT_VARIABLE USER_AGENT_DISTRO_CODENAME OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND lsb_release -r -s OUTPUT_VARIABLE USER_AGENT_DISTRO_RELEASE  OUTPUT_STRIP_TRAILING_WHITESPACE)

    # handle distros that identify release as n/a...
    if (USER_AGENT_DISTRO_RELEASE STREQUAL "n/a")
        set(USER_AGENT_DISTRO_RELEASE "NA")
    endif()

else()
    if (APPLE)
        execute_process(COMMAND sw_vers -productName OUTPUT_VARIABLE USER_AGENT_DISTRO OUTPUT_STRIP_TRAILING_WHITESPACE)
        #execute_process(COMMAND "grep -oE 'SOFTWARE LICENSE AGREEMENT FOR OS X.*[A-Z]' '/System/Library/CoreServices/Setup Assistant.app/Contents/Resources/en.lproj/OSXSoftwareLicense.rtf' | awk -F 'OS X ' '{print $NF}'" OUTPUT_VARIABLE USER_AGENT_DISTRO_CODENAME OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(USER_AGENT_DISTRO_CODENAME "El Capitan")
        execute_process(COMMAND sw_vers -productVersion OUTPUT_VARIABLE USER_AGENT_DISTRO_RELEASE  OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        message(AUTHOR_WARNING "lsb_release not found!")
    endif()
endif()

find_program(GIT_EXECUTABLE NAMES git)

# Git tags are optional. "git describe --tags" returns no version for repos
# without reachable tags; identify each build by the actual source commit.
set(RTNG_GIT_VERSION "v${PROJECT_VERSION}-source")
if (GIT_EXECUTABLE)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${PROJECT_SOURCE_DIR}" rev-parse --verify --short=12 HEAD
        RESULT_VARIABLE RTNG_GIT_STATUS
        OUTPUT_VARIABLE RTNG_GIT_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if (RTNG_GIT_STATUS EQUAL 0 AND RTNG_GIT_REVISION)
        set(RTNG_GIT_VERSION "v${PROJECT_VERSION}-g${RTNG_GIT_REVISION}")
        # Report tracked source changes, ignoring build output and other untracked files.
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" -C "${PROJECT_SOURCE_DIR}" diff --quiet HEAD --
            RESULT_VARIABLE RTNG_GIT_DIRTY
            ERROR_QUIET
        )
        if (RTNG_GIT_DIRTY EQUAL 1)
            string(APPEND RTNG_GIT_VERSION "-dirty")
        endif()
    endif()
endif()
message(STATUS "RadioTray-NG build reference: ${RTNG_GIT_VERSION}")

if (USER_AGENT_DISTRO AND USER_AGENT_DISTRO_RELEASE AND USER_AGENT_DISTRO_CODENAME)
    set(RTNG_USER_AGENT "${CMAKE_PROJECT_NAME}/${PROJECT_VERSION} (${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}; ${USER_AGENT_DISTRO}/${USER_AGENT_DISTRO_RELEASE} (${USER_AGENT_DISTRO_CODENAME}))")
else()
    set(RTNG_USER_AGENT "${CMAKE_PROJECT_NAME}/${PROJECT_VERSION} (${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}; Unknown)")
endif()


configure_file(${PROJECT_SOURCE_DIR}/cmake/rtng_user_agent.hpp.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rtng_user_agent.hpp.tmp)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rtng_user_agent.hpp.tmp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rtng_user_agent.hpp)
execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rtng_user_agent.hpp.tmp)

# message(STATUS "user-agent: ${RTNG_USER_AGENT}")
