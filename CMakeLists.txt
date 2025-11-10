cmake_minimum_required(VERSION 3.17)
project(main C)
set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
	#set(CMAKE_BUILD_TYPE Debug)
endif()

set(INC_PATH  ${PROJECT_SOURCE_DIR}/include)
set(SRC_PATH  ${PROJECT_SOURCE_DIR}/src)

add_executable(${PROJECT_NAME} main.c)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

target_include_directories(${PROJECT_NAME} PUBLIC ${INC_PATH})
target_compile_options(${PROJECT_NAME}
	PRIVATE
		-std=c11
		-O3 -ffast-math -funroll-loops -march=native -mtune=native
		-funswitch-loops -ftree-vectorize -fivopts -fmodulo-sched -flto

		-Wall -Wextra -Wno-unused-function -pedantic -pipe
		-Wno-implicit-function-declaration
		-ftrapv -fstack-protector-all -fstack-protector-strong
		-fno-strict-aliasing
		-DNDebug
)
