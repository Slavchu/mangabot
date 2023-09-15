cmake_minimum_required(VERSION 3.14.0)
project(mangabot)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(JSON_BuildTests OFF CACHE INTERNAL "")


add_executable(mangabot main.cpp)



#Adding libraries
add_subdirectory(thread_manager)
target_link_libraries(mangabot slavchoo_thread_manager)


add_subdirectory(bot_api)
target_link_libraries(mangabot bot_api)

add_subdirectory(database)
target_link_libraries(mangabot mangabot_database)

find_package(CURL REQUIRED)
target_link_libraries(mangabot ${CURL_LIBRARIES})