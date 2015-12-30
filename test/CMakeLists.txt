cmake_minimum_required(VERSION 2.8.9)
project(ls-game-file-test)

set(CMAKE_C_COMPILER_ID "GNU")
set(CMAKE_CXX_COMPILER_ID "GNU")
set(CMAKE_COMPILER_IS_GNUCXX TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SEP "/")

set(BASE_DIR "${CMAKE_BINARY_DIR}${SEP}..")
set(TESTSRC_DIR "${CMAKE_BINARY_DIR}")
set(SOURCE_DIR "${BASE_DIR}${SEP}src")
set(INCLUDE_DIR "${BASE_DIR}${SEP}inc")
set(BUILD_DIR "${CMAKE_BINARY_DIR}${SEP}bin")

file(GLOB SOURCES "${SOURCE_DIR}${SEP}*.cpp")
file(GLOB TESTSRCS "${TESTSRC_DIR}${SEP}*.cpp")
string(REGEX REPLACE "${TESTSRC_DIR}${SEP}([a-z-]+)\\.cpp" "\\1" BINARIES "${TESTSRCS}")

message(STATUS "SOURCE_DIR=${SOURCE_DIR}")
message(STATUS "INCLUDE_DIR=${INCLUDE_DIR}")
message(STATUS "TESTSRC_DIR=${TESTSRC_DIR}")
message(STATUS "SOURCES=${SOURCES}")
message(STATUS "BINARIES=${BINARIES}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})
include_directories("${INCLUDE_DIR}")

#
# loop foreach target
#

foreach(MAIN_EXE IN LISTS BINARIES)

set(MAIN_SRC "${TESTSRC_DIR}/${MAIN_EXE}.cpp")

message(STATUS "${MAIN_EXE}: ${MAIN_SRC} ${SOURCES}")

add_executable("${MAIN_EXE}" "${MAIN_SRC}" "${SOURCES}")

endforeach(MAIN_EXE)



