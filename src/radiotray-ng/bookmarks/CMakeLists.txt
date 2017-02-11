FILE(GLOB SRC_FILES "*.cpp")

add_library(bookmarks STATIC ${SRC_FILES})
target_link_libraries(bookmarks)
target_include_directories(bookmarks PRIVATE ${JSONCPP_INCLUDE_DIRS})
