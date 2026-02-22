cmake_minimum_required(VERSION 3.20)
project(pypp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pypp src/main.cpp)
if(WIN32)
  target_link_libraries(pypp PRIVATE gdiplus Ws2_32)
endif()

include(CTest)
if(BUILD_TESTING)
  add_test(
    NAME run_hello
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/hello.pypp
  )
  add_test(
    NAME compile_alias_hello
    COMMAND pypp compile ${CMAKE_SOURCE_DIR}/examples/hello.pypp --out ${CMAKE_BINARY_DIR}/artifacts
  )
  add_test(
    NAME run_bytecode_hello
    COMMAND pypp run-bytecode ${CMAKE_SOURCE_DIR}/examples/hello_bytecode.ppbc
  )
  add_test(
    NAME run_graphics
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/graphics.pypp
  )
  add_test(
    NAME run_gx3d_frame
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/gx3d_frame.pypp
  )
  add_test(
    NAME run_random_noise
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/random_noise.pypp
  )
  add_test(
    NAME run_math_numpy
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/math_numpy_demo.pypp
  )
  add_test(
    NAME run_torch
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/torch_demo.pypp
  )
endif()
