cmake_minimum_required(VERSION 3.20)

project(CometSuite CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT MSVC)
    message(FATAL_ERROR "現在は MSVC のみ対応しています。Makefile を使ってください。")
endif()

# Conan の CMakeDeps/CMakeToolchain が生成した設定を利用する
find_package(wxWidgets CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)

# リポジトリ内の src/include (サブモジュール)
set(COMET_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} WIN32)

target_sources(${PROJECT_NAME} PRIVATE
    host/main.cpp
    host/ui/mainframe.cpp
    host/ipc/ipc_server.cpp
    host/resource.rc
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/host
    ${CMAKE_CURRENT_SOURCE_DIR}/common
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

# resource.rc (wx/msw/wx.rc) が DPI 対応マニフェストを埋め込むため、
# MSVC リンカ既定のマニフェスト自動生成を無効化する (競合すると DPI 対応が無効になり、
# 高 DPI 環境でぼやける原因になる)。
target_link_options(${PROJECT_NAME} PRIVATE "/MANIFEST:NO")

target_link_libraries(${PROJECT_NAME} PRIVATE
    wxWidgets::wxWidgets
    opencv::opencv
    Eigen3::Eigen
    ws2_32
)
