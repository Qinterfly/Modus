
cmake_minimum_required(VERSION 3.22)

# Set parameters

## Data
set(APP_NAME Modus)
set(APP_CREATOR "Pavel Lakiza")
## Version
set(VERSION_MAJOR 1)
set(VERSION_MINOR 7)
set(VERSION_PATCH 1)
set(VERSION_FULL ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
## Directories
set(EXAMPLES_DIR ${CMAKE_SOURCE_DIR}/examples)
set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/resources)
set(TRANSLATIONS_DIR ${RESOURCES_DIR}/translations)
set(TEMPORARY_DIR ${CMAKE_BINARY_DIR})
set(ROOT_INSTALL_DIR ${CMAKE_SOURCE_DIR}/install)
## Files
set(RESOURCES_FILE ${RESOURCES_DIR}/resources.qrc)
set(INFO_FILE ${RESOURCES_DIR}/info.rc)

# Set language standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files")
configure_file(cmake/config.h.in config.h @ONLY)

# Specify project options
project(modus LANGUAGES CXX)

# Add dependencies
add_subdirectory(lib)

# Add project elements
add_subdirectory(src)
add_subdirectory(tests)
