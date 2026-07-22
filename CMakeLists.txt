cmake_minimum_required(VERSION 3.19)
project(elcs LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.h
    src/*.ui
)

qt_add_executable(elcs
    WIN32 MACOSX_BUNDLE
    ${SOURCES}
    main.cpp
)

target_include_directories(elcs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

set_target_properties(elcs PROPERTIES
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/src/ui"
)

target_link_libraries(elcs
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS elcs
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET elcs
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
