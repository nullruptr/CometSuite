# CometSuite.exe (host) のビルド (MSVC + Conan)
# 初回: このディレクトリで poetry install
# "Developer PowerShell for VS 2022" から実行すること。
#   make compile        Release ビルド
#   make compile-debug  Debug ビルド
#   make compdb         clangd 用 compile_commands.json を生成

RELEASE_CONAN_DIR := build/release
DEBUG_CONAN_DIR   := build/debug
COMPDB_CONAN_DIR  := build/compiledb-conan
COMPDB_DIR        := build/compiledb

.PHONY: compile compile-debug compdb clean

compile:
	poetry run conan install . --output-folder=$(RELEASE_CONAN_DIR) \
		-pr:h=profiles/msvc-release -pr:b=profiles/msvc-build --build=missing
	cmake -S . -B $(RELEASE_CONAN_DIR)/build -G "Visual Studio 17 2022" -A x64 -T v143 \
		-DCMAKE_TOOLCHAIN_FILE=$(RELEASE_CONAN_DIR)/build/generators/conan_toolchain.cmake \
		-DCMAKE_POLICY_DEFAULT_CMP0091=NEW
	cmake --build $(RELEASE_CONAN_DIR)/build --config Release

compile-debug:
	poetry run conan install . --output-folder=$(DEBUG_CONAN_DIR) \
		-pr:h=profiles/msvc-debug -pr:b=profiles/msvc-build --build=missing
	cmake -S . -B $(DEBUG_CONAN_DIR)/build -G "Visual Studio 17 2022" -A x64 -T v143 \
		-DCMAKE_TOOLCHAIN_FILE=$(DEBUG_CONAN_DIR)/build/generators/conan_toolchain.cmake \
		-DCMAKE_POLICY_DEFAULT_CMP0091=NEW
	cmake --build $(DEBUG_CONAN_DIR)/build --config Debug

# Visual Studio ジェネレータは compile_commands.json を出せないため Ninja で別途生成する。
# conan_toolchain.cmake は CMAKE_GENERATOR_PLATFORM=x64 を強制し Ninja と非互換のため、
# generator=Ninja を指定した専用の toolchain を別 output-folder に生成する。
# NOTE: cl.exe / ninja.exe が PATH に必要。
compdb:
	poetry run conan install . --output-folder=$(COMPDB_CONAN_DIR) \
		-pr:h=profiles/msvc-release -pr:b=profiles/msvc-build --build=missing \
		-c tools.cmake.cmaketoolchain:generator=Ninja
	cmake -E rm -rf $(COMPDB_DIR)
	cmake -S . -B $(COMPDB_DIR) -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE=$(COMPDB_CONAN_DIR)/build/Release/generators/conan_toolchain.cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(COMPDB_DIR)
	cmake -E copy $(COMPDB_DIR)/compile_commands.json ./compile_commands.json

clean:
	cmake -E rm -rf build
