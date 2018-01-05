cmake_minimum_required(VERSION 2.6)

set(srcs test)

add_executable(test ${srcs})

if (UNIX)
  set( CMAKE_CXX_FLAGS "-fno-stack-protector" )
else ()
 MESSAGE( STATUS "Not Unix!!!!!!!" )
 set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GS-")
endif ()
