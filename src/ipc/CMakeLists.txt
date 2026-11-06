cmake_minimum_required(VERSION 3.20)

project(IPC CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT MSVC)
    message(FATAL_ERROR "現在は MSVC のみ対応しています。Makefile を使ってください。")
endif()

find_package(wxWidgets CONFIG REQUIRED)

add_executable(${PROJECT_NAME} WIN32)

target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
    src/ui/mainframe.cpp
    src/resource.rc
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/wxCustomControls/src
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    $<$<CONFIG:Debug>:_DEBUG>
)

target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)

set_property(TARGET ${PROJECT_NAME} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# resource.rc (wx/msw/wx.rc) が DPI 対応マニフェストを埋め込むため、自動生成を無効化する
target_link_options(${PROJECT_NAME} PRIVATE "/MANIFEST:NO")

target_link_libraries(${PROJECT_NAME} PRIVATE
    wxWidgets::wxWidgets
    ws2_32
)
