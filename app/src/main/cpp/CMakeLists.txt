cmake_minimum_required(VERSION 3.22.1)

project("piperreader")

# This is a stub for future libpiper integration
add_library(
        piperreader
        SHARED
        native-lib.cpp)

find_library(
        log-lib
        log)

target_link_libraries(
        piperreader
        ${log-lib})
