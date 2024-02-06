cmake_minimum_required(VERSION 3.5)

project(simplePassManCmake VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6Sql REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Freetype REQUIRED)
find_package(PNG REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        window.cpp
        window.h
        window.ui
        dbmanager.h
        dbmanager.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(simplePassManCmake
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        dbmanager.h dbmanager.cpp
        cryptomanager.h cryptomanager.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET simplePassManCmake APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(simplePassManCmake SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(simplePassManCmake
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(simplePassManCmake PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(simplePassManCmake PRIVATE Qt${QT_VERSION_MAJOR}::Sql)
target_link_libraries(simplePassManCmake PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(simplePassManCmake PRIVATE Freetype::Freetype)
target_link_libraries(simplePassManCmake PRIVATE PNG::PNG)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.

include(GNUInstallDirs)
include(CPack)

install(TARGETS simplePassManCmake
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(simplePassManCmake)
endif()
