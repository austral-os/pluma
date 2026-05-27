cmake_minimum_required(VERSION 3.20)
project(pluma-app VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(HORIZON REQUIRED IMPORTED_TARGET horizon)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(pluma-app ${SOURCES})

target_include_directories(pluma-app PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CAIRO_INCLUDE_DIRS}
    ${FONTCONFIG_INCLUDE_DIRS}
)

target_link_directories(pluma-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib-pluma/build)

target_link_libraries(pluma-app PRIVATE 
    PkgConfig::HORIZON
    pluma
    ${CAIRO_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
)

target_compile_definitions(pluma-app PRIVATE APP_VERSION="${PROJECT_VERSION}")

set_target_properties(pluma-app PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/../lib-pluma/build"
)



