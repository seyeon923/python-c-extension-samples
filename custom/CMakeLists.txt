project(custom)

add_library(custom custom.c)

target_link_libraries(custom PRIVATE Python3::Python)