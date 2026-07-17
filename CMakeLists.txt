cmake_minimum_required(VERSION 3.20)
project(pluma-writer VERSION 0.8.12 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(HORIZON REQUIRED IMPORTED_TARGET horizon)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
pkg_check_modules(ICU REQUIRED icu-uc icu-i18n)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(pluma-writer ${SOURCES})

target_include_directories(pluma-writer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib-pluma/include
    ${CAIRO_INCLUDE_DIRS}
    ${FONTCONFIG_INCLUDE_DIRS}
    ${ICU_INCLUDE_DIRS}
)

target_link_directories(pluma-writer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib-pluma/build)

target_link_libraries(pluma-writer PRIVATE 
    PkgConfig::HORIZON
    pluma
    ${CAIRO_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${ICU_LIBRARIES}
)

target_compile_definitions(pluma-writer PRIVATE APP_VERSION="${PROJECT_VERSION}")


# --- Installation ---
include(GNUInstallDirs)

set_target_properties(pluma-writer PROPERTIES
    INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/pluma-writer"
    BUILD_WITH_INSTALL_RPATH FALSE
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

install(TARGETS pluma-writer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Empaquetamos libpluma
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib-pluma/build/
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pluma-writer
    FILES_MATCHING PATTERN "libpluma.so*"
)

# Empaquetamos libhorizon
pkg_get_variable(HORIZON_LIBDIR horizon libdir)
install(DIRECTORY ${HORIZON_LIBDIR}/
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pluma-writer
    FILES_MATCHING PATTERN "libhorizon*.so*"
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

install(FILES assets/color-scheme.json
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/horizon/apps/pluma-writer
)

add_custom_command(TARGET pluma-writer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/locales
    $<TARGET_FILE_DIR:pluma-writer>/locales
    COMMENT "Copying app locales to build directory"
)

add_custom_command(TARGET pluma-writer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:pluma-writer>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/color-scheme.json
    $<TARGET_FILE_DIR:pluma-writer>/assets/color-scheme.json
    COMMENT "Copying app color scheme to build directory"
)

install(DIRECTORY assets/icons/scalable/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable
)

install(DIRECTORY assets/dictionaries/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pluma-writer/dictionaries
)

add_custom_command(TARGET pluma-writer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/dictionaries
    $<TARGET_FILE_DIR:pluma-writer>/dictionaries
    COMMENT "Copying app dictionaries to build directory"
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
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON) # Calcula dependencias como cairo o icu automáticamente
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libnuspell5")
# Ya no dependemos de paquetes externos para libpluma y libhorizon porque van adentro
# set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpluma (>= 0.1.0), libhorizon")
set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

# --- Testing ---
# Standalone test executable (no external dependencies, no network).
option(BUILD_TESTS "Build pluma-writer unit tests" ON)

if(BUILD_TESTS)
    enable_testing()

    add_executable(pluma_writer_tests
        tests/test_list_paragraph.cpp
    )

    add_executable(pluma_scroll_sync_tests
        tests/test_scroll_sync.cpp
    )

    target_include_directories(pluma_writer_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_include_directories(pluma_scroll_sync_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib-pluma/include
    )

    target_link_directories(pluma_scroll_sync_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib-pluma/build)

    target_link_libraries(pluma_scroll_sync_tests PRIVATE
        PkgConfig::HORIZON
        pluma
    )

    add_test(NAME paragraph_tests COMMAND pluma_writer_tests)
    add_test(NAME scroll_sync_tests COMMAND pluma_scroll_sync_tests)
endif()

include(CPack)
