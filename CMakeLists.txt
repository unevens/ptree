cmake_minimum_required(VERSION 3.5)
project(ptree)

if (WIN32)
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-pie -no-pie")
endif()

set (CMAKE_BUILD_TYPE Release)

aux_source_directory(src SRC_LIST)
file (GLOB headers "src/*.h")

if (WIN32)
  add_executable(ptree-test "src/ptree.c" "src/test.cpp" ${headers})
  add_executable(ptree-example "src/ptree.c" "src/example.c" ${headers})
  add_executable(ptree-bench "src/ptree.c" "src/benchmark.cpp" ${headers})
else()
  add_executable(ptree-test "src/ptree.c" "src/test.cpp")
  add_executable(ptree-example "src/ptree.c" "src/example.c")
  target_link_libraries(ptree-example m)
  add_executable(ptree-bench "src/ptree.c" "src/benchmark.cpp")
endif()
