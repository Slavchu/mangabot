cmake_minimum_required(VERSION 3.14.0)
if (POLICY CMP0135) 
    cmake_policy(SET CMP0135 NEW)
endif()

add_library(telegraph_api ${CMAKE_CURRENT_SOURCE_DIR}/src/telegraph.cpp)
target_include_directories(telegraph_api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(CURL REQUIRED)
target_link_libraries(telegraph_api ${CURL_LIBRARIES})

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)


target_link_libraries(telegraph_api nlohmann_json::nlohmann_json)