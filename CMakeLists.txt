cmake_minimum_required(VERSION 3.19)
project(elcs LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(elcs
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    circuitworkspace.h circuitworkspace.cpp
    dotgridscene.h dotgridscene.cpp
    componentholder.h componentholder.cpp

    component.h component.cpp
    scheduler.h scheduler.cpp
    filehelper.h filehelper.cpp
    componenttoolbox.h componenttoolbox.cpp
    projectmanager.h projectmanager.cpp
    componentToolboxElement.ui


)

target_include_directories(elcs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
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
