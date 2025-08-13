# 交叉编译工具链
set(TOOLCHAIN_PATH "/opt/tiger/riscv-toolchain/")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/bin/clang")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/bin/clang++")

set(CMAKE_SYSROOT "${TOOLCHAIN_PATH}/sysroot")
set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")
