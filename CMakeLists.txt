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
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-pluma/include
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


# --- Installation ---
include(GNUInstallDirs)
install(TARGETS pluma-app
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES pluma.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

install(DIRECTORY assets/icons/scalable/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable
)

# --- Packaging (CPack) ---
set(CPACK_PACKAGE_NAME "pluma")
set(CPACK_PACKAGE_VENDOR "hdrdevs")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Pluma Rich Text Editor")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "horaciodrs@gmail.com")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "hdrdevs")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpluma (>= 0.1.0), libhorizon")
set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

include(CPack)

