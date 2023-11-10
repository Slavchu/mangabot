cmake_minimum_required(VERSION 3.27.0)
project(mangabot)

set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=16000000")
set(CMAKE_CXX_STANDARD 17)

set(JSON_BuildTests OFF CACHE INTERNAL "")


add_executable(mangabot main.cpp bot.hpp bot.cpp)



#Adding libraries
add_subdirectory(thread_manager)
target_link_libraries(mangabot slavchoo_thread_manager)

add_subdirectory(telegraph_api)
target_link_libraries(mangabot telegraph_api)

add_subdirectory(bot_api)
target_link_libraries(mangabot bot_api)

add_subdirectory(database)
target_link_libraries(mangabot mangabot_database)


add_subdirectory(logger)
target_link_libraries(mangabot logger)
