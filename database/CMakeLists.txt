cmake_minimum_required(VERSION 3.14.0)

add_library(mangabot_database src/database.cpp include/database.hpp)
target_include_directories(mangabot_database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(mangabot_database PRIVATE nlohmann_json::nlohmann_json)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
 
include(FindPkgConfig)
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)
 
foreach(FLAG ${LIBMYSQLCLIENT_CFLAGS_OTHER})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
endforeach()
 
link_directories(${LIBMYSQLCLIENT_LIBRARY_DIRS})
 
target_link_libraries(mangabot_database PUBLIC mysqlcppconn)
target_include_directories(mangabot_database PUBLIC ${LIBMYSQLCLIENT_INCLUDE_DIRS})
target_link_libraries(mangabot_database PRIVATE ${LIBMYSQLCLIENT_LIBRARIES})
