# 顶层cmakelist文件，以下的设置将被子工程所继承使用
cmake_minimum_required(VERSION 3.0.0)

# 保存一些通用的配置
# set(CMAKE_VERBOSE_MAKEFILE on)   # 开启输出编译详细过程的提示

# gcc工具链前缀，根据实际情况做不同的设置
# 目前mac和win上使用x86_64-elf-
if (CMAKE_HOST_WIN32 OR CMAKE_HOST_APPLE)
    set(TOOL_PREFIX  "x86_64-elf-")
elseif (CMAKE_HOST_WIN32)
    set(TOOL_PREFIX  "x86_64-linux-gnu-")    # 待补充
endif ()

# set(TOOL_PREFIX  "x86_64-elf-")
# set(TOOL_PREFIX  "i686-elf-")

# C编译器与参数配置
set(CMAKE_C_COMPILER "${TOOL_PREFIX}gcc")
set(CMAKE_C_FLAGS "-g -c -O0 -m32 -fno-pie -fno-stack-protector -fno-asynchronous-unwind-tables")

# 汇编器与参数配置
set(CMAKE_ASM_COMPILER "${TOOL_PREFIX}gcc")
set(CMAKE_ASM_FLAGS "-m32 -g")
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS "asm")

# 链接器工具
set(LINKER_TOOL "${TOOL_PREFIX}ld")

# 其它工具
set(OBJCOPY_TOOL "${TOOL_PREFIX}objcopy")
set(OBJDUMP_TOOL "${TOOL_PREFIX}objdump")
set(READELF_TOOL "${TOOL_PREFIX}readelf")

# 针对x86 os上的网络配置
add_definitions(-DNET_DRIVER_RTL8139)
add_definitions(-DSYS_PLAT_X86OS)


# 工程，启用C语言和汇编语言
project(os LANGUAGES C)  
enable_language(ASM)

# 头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/source
    ${PROJECT_SOURCE_DIR}/source/kernel/include
    ${PROJECT_SOURCE_DIR}/source/kernel/net/net/
    ${PROJECT_SOURCE_DIR}/source/kernel/net/plat
    ${PROJECT_SOURCE_DIR}/newlib/i686-elf/include
)

# 底层的若干子项目：含内核及应用程序
add_subdirectory(./source/boot)
add_subdirectory(./source/loader)
add_subdirectory(./source/kernel)
add_subdirectory(./source/applib)
add_subdirectory(./source/shell)
add_subdirectory(./source/init)
add_subdirectory(./source/loop)
add_subdirectory(./source/snake)
add_subdirectory(./source/ping)
add_subdirectory(./source/tftp)

# 添加编译依赖，先生成app库，再生成kernel和shell
# 不加则cmake则可能先编译shell和kernel，而缺少libapp，导致编译错误
# add_dependencies(init app)
add_dependencies(shell app)
add_dependencies(kernel app)
# add_dependencies(loop app)
# add_dependencies(kernel init)
