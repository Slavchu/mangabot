cmake_minimum_required(VERSION 3.14.0)

add_library(logger src/logger.cpp include/logger.hpp)
target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
