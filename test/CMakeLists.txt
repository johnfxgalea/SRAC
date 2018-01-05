cmake_minimum_required(VERSION 2.6)

set(srcs test)

add_compile_options(-fno-stack-protector)

add_executable(test ${srcs})

