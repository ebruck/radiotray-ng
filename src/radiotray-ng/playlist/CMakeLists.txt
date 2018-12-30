add_library(playlist STATIC
    asf_decoder.cpp
    asf_decoder.hpp
    asx_decoder.cpp
    asx_decoder.hpp
    m3u_decoder.cpp
    m3u_decoder.hpp
    pls_decoder.cpp
    pls_decoder.hpp
    ram_decoder.cpp
    ram_decoder.hpp
    xspf_decoder.cpp
    xspf_decoder.hpp
    playlist_downloader.cpp
    playlist_downloader.hpp)

target_link_libraries(playlist)
add_dependencies(playlist user-agent)
target_include_directories(playlist PRIVATE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
