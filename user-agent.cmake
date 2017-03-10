# todo: enhance player to detect the distro information at runtime

find_program(LSB_RELEASE_EXECUTABLE NAMES lsb_release)

if (LSB_RELEASE_EXECUTABLE)
    execute_process(COMMAND lsb_release -i -s OUTPUT_VARIABLE USER_AGENT_DISTRO          OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND lsb_release -c -s OUTPUT_VARIABLE USER_AGENT_DISTRO_CODENAME OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND lsb_release -r -s OUTPUT_VARIABLE USER_AGENT_DISTRO_RELEASE  OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    if (APPLE)
        execute_process(COMMAND sw_vers -productName OUTPUT_VARIABLE USER_AGENT_DISTRO OUTPUT_STRIP_TRAILING_WHITESPACE)
        #execute_process(COMMAND "grep -oE 'SOFTWARE LICENSE AGREEMENT FOR OS X.*[A-Z]' '/System/Library/CoreServices/Setup Assistant.app/Contents/Resources/en.lproj/OSXSoftwareLicense.rtf' | awk -F 'OS X ' '{print $NF}'" OUTPUT_VARIABLE USER_AGENT_DISTRO_CODENAME OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(USER_AGENT_DISTRO_CODENAME "El Capitan")
        execute_process(COMMAND sw_vers -productVersion OUTPUT_VARIABLE USER_AGENT_DISTRO_RELEASE  OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        set(USER_AGENT_DISTRO          "unknown")
        set(USER_AGENT_DISTRO_CODENAME "unknown")
        set(USER_AGENT_DISTRO_RELEASE  "unknown")
    endif()
endif()

find_program(GIT_EXECUTABLE NAMES git)

if (GIT_EXECUTABLE)
    execute_process(COMMAND git describe --tags --dirty OUTPUT_VARIABLE RTNG_GIT_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    set(RTNG_GIT_VERSION "v${PROJECT_VERSION}-unknown")
endif()

set(RTNG_USER_AGENT "${CMAKE_PROJECT_NAME}/${PROJECT_VERSION} (${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}; ${USER_AGENT_DISTRO}/${USER_AGENT_DISTRO_RELEASE} (${USER_AGENT_DISTRO_CODENAME}))")
configure_file(${PROJECT_SOURCE_DIR}/rtng_user_agent.hpp.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rtng_user_agent.hpp.tmp)

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rtng_user_agent.hpp.tmp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rtng_user_agent.hpp)
execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rtng_user_agent.hpp.tmp)

message(STATUS "user-agent: ${RTNG_USER_AGENT}")