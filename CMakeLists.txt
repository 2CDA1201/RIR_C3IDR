cmake_minimum_required(VERSION 4.1.1)
project(RIR_C3IDR VERSION 1.0.0 LANGUAGES CXX)

# Use C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection and library setup
if(WIN32)
    message(STATUS "Building for Windows")
    set(PLATFORM_LIBRARIES wininet)
else()
    message(STATUS "Building for Unix-like system")
    find_package(CURL REQUIRED)
    set(PLATFORM_LIBRARIES ${CURL_LIBRARIES})
    set(PLATFORM_INCLUDE_DIRS ${CURL_INCLUDE_DIRS})
endif()

# Define source files
set(SOURCE_FILES
    src/main.cpp
    src/buffered_reader.cpp
    src/ip_utils.cpp
    src/rir_fetcher.cpp
    src/http_client.cpp
)

# Create executable
add_executable(RIR_C3IDR ${SOURCE_FILES})

# Configure include directories
target_include_directories(RIR_C3IDR PRIVATE include)
if(PLATFORM_INCLUDE_DIRS)
    target_include_directories(RIR_C3IDR PRIVATE ${PLATFORM_INCLUDE_DIRS})
endif()

# Link libraries
target_link_libraries(RIR_C3IDR PRIVATE ${PLATFORM_LIBRARIES})

# Compiler-specific settings
if(MSVC)
    # Settings for Visual Studio
    target_compile_definitions(RIR_C3IDR PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(RIR_C3IDR PRIVATE /W4)
    add_compile_options("/utf-8")
else()
    # Settings for GCC/Clang
    target_compile_options(RIR_C3IDR PRIVATE -Wall -Wextra -pedantic)
endif()

# Debug build settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(RIR_C3IDR PRIVATE DEBUG)
endif()

# Installation settings
install(TARGETS RIR_C3IDR
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)

# CPack configuration
set(CPACK_PACKAGE_NAME "RIR_C3IDR")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "RIR C3IDR - Fetch IP block data from RIR and convert to csv format")
set(CPACK_PACKAGE_VENDOR "RIR Tools")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
else()
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()

include(CPack)