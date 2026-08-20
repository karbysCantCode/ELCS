cmake_minimum_required(VERSION 3.19)
project(elcs LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

if(WIN32)

    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt.exe
        HINTS
            ${CMAKE_PREFIX_PATH}/bin
    )

    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "Found windeployqt: ${WINDEPLOYQT_EXECUTABLE}")

        add_custom_command(TARGET elcs POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE}
                --debug
                $<TARGET_FILE:elcs>
            COMMENT "Deploying Qt DLLs"
        )
    else()
        message(WARNING "windeployqt not found")
    endif()

endif()

target_include_directories(elcs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

target_compile_definitions(elcs PRIVATE COMPONENT_SAVE_VERSION_MAJOR=0)
target_compile_definitions(elcs PRIVATE COMPONENT_SAVE_VERSION_MINOR=3)
target_compile_definitions(elcs PRIVATE RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/res")
target_compile_definitions(elcs PRIVATE SAVE_FILE_EXTENSION=".csf")

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