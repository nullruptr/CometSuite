cmake_minimum_required(VERSION 3.20)

project(CometConnector_AviUtl2 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT MSVC)
    message(FATAL_ERROR "現在は MSVC のみ対応しています。Makefile を使ってください。")
endif()

# Conan の CMakeDeps/CMakeToolchain が生成した設定を利用する
find_package(wxWidgets CONFIG REQUIRED)

# リポジトリ内の src/include (サブモジュール)
set(COMET_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

add_library(${PROJECT_NAME} SHARED)

target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
    src/ui/mainframe.cpp
    src/ipc/host_client.cpp
    src/utils/dpi_patch.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".aux2"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${COMET_INCLUDE_DIR}/aviutl2_sdk_mirror/include
    ${COMET_INCLUDE_DIR}/wxCustomControls/src
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    $<$<CONFIG:Debug>:_DEBUG>
)

target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)

# 静的ランタイム (Conan プロファイルの compiler.runtime=static と合わせる)
set_property(TARGET ${PROJECT_NAME} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(${PROJECT_NAME} PRIVATE
    wxWidgets::wxWidgets
    comctl32
    ws2_32
)
