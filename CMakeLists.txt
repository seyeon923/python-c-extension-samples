cmake_minimum_required(VERSION 3.22)

project(python-c-extension-samples)

find_package(Python3 COMPONENTS Development REQUIRED)

add_subdirectory(custom)