# 指定 cmake 最低版本
cmake_minimum_required(VERSION 3.22.1)

# 指定项目名称和语言
project(tinyRenderer CXX)

# 设置 c++ 版本
set(CMAKE_CXX_STANDARD 17)

# 设置项目路径
set(HOME ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC "${HOME}/src")

message(${HOME})
message("cpp standard: " ${CMAKE_CXX_STANDARD})

# 递归检索目录下所有源文件
aux_source_directory(src SRCS)

# 指定头文件目录, SYSTEM表明是系统头文件
include_directories("${HOME}/include/")
include_directories(SYSTEM "/usr/include/")

# 生成可执行文件
add_executable(${PROJECT_NAME} ${SRCS})
