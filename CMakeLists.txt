cmake_minimum_required(VERSION 3.0.0)
project(aes VERSION 0.1.0)
set(ENTRYPOINT aes)

set(CMAKE_BUILD_TYPE Debug)

include_directories(src)
file(GLOB SOURCES "src/*.c")

add_executable(${ENTRYPOINT} ${SOURCES})
# target_link_libraries(${ENTRYPOINT} ...)

if (MSVC)
	target_compile_options(${ENTRYPOINT} PRIVATE /W4)
endif()
