cmake_minimum_required(VERSION 3.22)

project(python-c-extension-samples)

find_package(Python3 COMPONENTS Development REQUIRED)
link_libraries(Python3::Python)

add_subdirectory(extending_python_with_c_or_c++)
add_subdirectory(defining_extension_types)