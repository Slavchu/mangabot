cmake_minimum_required(VERSION 3.14.0)
project(mangabot)
add_executable(mangabot main.cpp)

set(JSON_BuildTests OFF CACHE INTERNAL "")


#Adding libraries
add_subdirectory(thread_manager)
target_link_libraries(mangabot slavchoo_thread_manager)


add_subdirectory(bot_api)
target_link_libraries(mangabot bot_api)

find_package(CURL REQUIRED)
target_link_libraries(mangabot ${CURL_LIBRARIES})