# SPDX-FileCopyrightText: 2024 Simon Gene Gottlieb
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required (VERSION 3.25)

project(demo LANGUAGES CXX DESCRIPTION "demonstration of CPM.dependencies")

include(cmake/CPM.cmake)
CPMAddPackage("gh:SGSSGene/cpm.dependencies@0.0.4")
CPMLoadDependenciesFile("${CMAKE_SOURCE_DIR}/cpm.dependencies")

add_executable(example example.cpp)
target_link_libraries(example PRIVATE nlohmann_json::nlohmann_json fmt::fmt cxxopts)
