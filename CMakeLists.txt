cmake_minimum_required(VERSION 3.16)

project(qrcode VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Svg)
qt_standard_project_setup()

qt_add_library(qrcode
    src/QrCodeGenerator.cpp
    src/qrcodegen/qrcodegen.cpp
)

target_include_directories(qrcode PUBLIC src/)

target_link_libraries(qrcode PRIVATE Qt6::Core Qt6::Gui Qt6::Svg)
