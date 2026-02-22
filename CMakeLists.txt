cmake_minimum_required(VERSION 3.20)
project(pypp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pypp src/main.cpp)
if(WIN32)
  target_link_libraries(pypp PRIVATE gdiplus)
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
    NAME run_fib
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/fib.pypp
  )
  add_test(
    NAME run_import_demo
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/import_demo.pypp
  )
  add_test(
    NAME run_import_objects_demo
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/import_objects_demo.pypp
  )
  add_test(
    NAME run_control_flow
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/control_flow.pypp
  )
  add_test(
    NAME run_graphics
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/graphics.pypp
  )
  add_test(
    NAME run_gx3d_frame
    COMMAND pypp run ${CMAKE_SOURCE_DIR}/examples/gx3d_frame.pypp
  )
endif()
