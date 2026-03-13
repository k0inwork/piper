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

# Wait, if we use Prefab, we do find_package(onnxruntime REQUIRED CONFIG)
# BUT the onnxruntime-android.aar does NOT contain a prefab/ folder!
# Let me look closely at the unzip output... yes, there is NO prefab/ folder in it!
# Therefore, find_package(onnxruntime REQUIRED CONFIG) will ALWAYS fail because Microsoft doesn't package it as Prefab.

# I MUST dynamically link the SO from the unzipped AAR in Gradle, or keep it in jniLibs.
