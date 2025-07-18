cmake_minimum_required(VERSION 3.16)

project(qrexample VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Svg)
qt_standard_project_setup()

add_subdirectory(../ ${CMAKE_BINARY_DIR}/qrcode)

qt_add_executable(qrexample
    main.cpp
    MainWindow.cpp
)

target_link_libraries(qrexample PRIVATE qrcode Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Svg)
