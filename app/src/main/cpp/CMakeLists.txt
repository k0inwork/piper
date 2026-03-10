cmake_minimum_required(VERSION 3.22.1)

project("piperreader")

# Add include directories for headers
include_directories(include)

# Tell the code to use the real implementation now
add_definitions(-DUSE_REAL_PIPER)

# Load our cross-compiled libpiper_android.so pre-built library
add_library(piper_android SHARED IMPORTED)
set_target_properties(piper_android PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libpiper_android.so)

# Load the onnxruntime pre-built library that libpiper depends on
add_library(onnxruntime SHARED IMPORTED)
set_target_properties(onnxruntime PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libonnxruntime.so)

add_library(
        piperreader
        SHARED
        native-lib.cpp)

find_library(
        log-lib
        log)

target_link_libraries(
        piperreader
        piper_android
        onnxruntime
        ${log-lib})
