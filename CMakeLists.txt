cmake_minimum_required(VERSION 3.20)
project(pluma-writer VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(HORIZON REQUIRED IMPORTED_TARGET horizon)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(pluma-writer ${SOURCES})

target_include_directories(pluma-writer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-pluma/include
    ${CAIRO_INCLUDE_DIRS}
    ${FONTCONFIG_INCLUDE_DIRS}
)

target_link_directories(pluma-writer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib-pluma/build)

target_link_libraries(pluma-writer PRIVATE 
    PkgConfig::HORIZON
    pluma
    ${CAIRO_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
)

target_compile_definitions(pluma-writer PRIVATE APP_VERSION="${PROJECT_VERSION}")


# --- Installation ---
include(GNUInstallDirs)
install(TARGETS pluma-writer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES pluma-writer.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

install(FILES pluma-writer.xml
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mime/packages
)

install(DIRECTORY locales/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/horizon/apps/pluma-writer/locales
)

add_custom_command(TARGET pluma-writer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/locales
    $<TARGET_FILE_DIR:pluma-writer>/locales
    COMMENT "Copying app locales to build directory"
)

install(DIRECTORY assets/icons/scalable/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable
)

# --- Packaging (CPack) ---
set(CPACK_PACKAGE_NAME "pluma-writer")
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

