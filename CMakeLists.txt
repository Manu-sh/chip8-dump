cmake_minimum_required(VERSION 3.17)
project(main C)
set(CMAKE_C_STANDARD 11)

set(INC_PATH  ${PROJECT_SOURCE_DIR}/include)
set(SRC_PATH  ${PROJECT_SOURCE_DIR}/src)

# my git submodule
#set(MY_COLLECTION_PATH ${PROJECT_SOURCE_DIR}/my_collections/src)

add_executable(${PROJECT_NAME} main.c)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

target_include_directories(${PROJECT_NAME} PUBLIC ${INC_PATH})
#target_include_directories(${PROJECT_NAME} PUBLIC ${MY_COLLECTION_PATH})

# generate lifo_u16.h
#execute_process(
#	COMMAND bash ${MY_COLLECTION_PATH}/inlined/lifo/generics-gen-lifo.sh u16
#	WORKING_DIRECTORY ${MY_COLLECTION_PATH}/inlined/lifo
#)

target_compile_options(${PROJECT_NAME}
	PRIVATE
		-std=c11 -O0
		-Wall -Wextra -Wno-unused-function -pedantic -pipe
		-Wno-implicit-function-declaration
		-ftrapv -fstack-protector-all -fstack-protector-strong
		-fno-strict-aliasing
)
