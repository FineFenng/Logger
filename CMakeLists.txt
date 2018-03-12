cmake_minimum_required(VERSION 3.8)
project(Logger)

set(CMAKE_CXX_STANDARD 11)


set(SOURCE_FILES main.cpp Logger.cpp Logger.h LogStream.cpp LogStream.h LogUtility.cpp LogUtility.h AsyncLogging.cpp AsyncLogging.h LogFile.cpp LogFile.h)

include_directories(~/MyInclude)
add_executable(Logger ${SOURCE_FILES})