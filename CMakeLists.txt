cmake_minimum_required(VERSION 3.17)
project(main C)
set(CMAKE_C_STANDARD 11)

set(INC_PATH  ${PROJECT_SOURCE_DIR}/include)
set(SRC_PATH  ${PROJECT_SOURCE_DIR}/src)

add_executable(${PROJECT_NAME} main.c)

find_package(raylib 2.0 REQUIRED)
target_link_libraries(${PROJECT_NAME} raylib)

target_include_directories(${PROJECT_NAME} PUBLIC ${INC_PATH})
target_compile_options(${PROJECT_NAME}
	PRIVATE
		-std=c11 -O0
		-Wall -Wextra -Wno-unused-function -pedantic -pipe
		-Wno-implicit-function-declaration
		-ftrapv -fstack-protector-all -fstack-protector-strong
		-fno-strict-aliasing
)
