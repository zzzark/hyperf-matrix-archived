#
# Created by zrk on 2023/10/20.
#

cmake_minimum_required(VERSION 3.26)
project(hyperf_matrix)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DAVX2_ENABLED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")


#set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")


add_compile_options(-fauto-inc-dec -fbranch-count-reg -fcombine-stack-adjustments -fcompare-elim -fcprop-registers -fdce -fdefer-pop -fdse -fforward-propagate -fguess-branch-probability
-fif-conversion -fif-conversion2 -finline-functions-called-once -fipa-modref -fipa-profile -fipa-pure-const -fipa-reference -fipa-reference-addressable -fmerge-constants
-fmove-loop-invariants -fmove-loop-stores -fomit-frame-pointer -freorder-blocks -fshrink-wrap -fshrink-wrap-separate -fsplit-wide-types -fssa-backprop -fssa-phiopt -ftree-bit-ccp
-ftree-ccp -ftree-ch -ftree-coalesce-vars -ftree-copy-prop -ftree-dce -ftree-dominator-opts -ftree-dse -ftree-forwprop -ftree-fre -ftree-phiprop -ftree-pta -ftree-scev-cprop -ftree-sink -ftree-slsr -ftree-sra -ftree-ter -funit-at-a-time)

add_compile_options(
        -falign-functions  -falign-jumps
        -falign-labels  -falign-loops -fcaller-saves -fcode-hoisting -fcrossjumping -fcse-follow-jumps  -fcse-skip-blocks -fdelete-null-pointer-checks -fdevirtualize  -fdevirtualize-speculatively -fexpensive-optimizations -ffinite-loops -fgcse  -fgcse-lm -fhoist-adjacent-loads -finline-functions -finline-small-functions
        -findirect-inlining -fipa-bit-cp  -fipa-cp  -fipa-icf -fipa-ra  -fipa-sra  -fipa-vrp -fisolate-erroneous-paths-dereference -flra-remat -foptimize-sibling-calls -foptimize-strlen -fpartial-inlining -fpeephole2 -freorder-blocks-algorithm=stc -freorder-functions -frerun-cse-after-loop -fschedule-insns  -fschedule-insns2 -fsched-interblock  -fsched-spec -fstore-merging
        -fstrict-aliasing -fthread-jumps -ftree-builtin-call-dce -ftree-loop-vectorize -ftree-pre -ftree-slp-vectorize -ftree-switch-conversion  -ftree-tail-merge -ftree-vrp -fvect-cost-model=very-cheap
)

add_compile_options(
        -fgcse-after-reload -fipa-cp-clone -floop-interchange -floop-unroll-and-jam -fpeel-loops -fpredictive-commoning -fsplit-loops
        -fsplit-paths -ftree-loop-distribution -ftree-partial-pre -funswitch-loops -fvect-cost-model=dynamic -fversion-loops-for-strides
)

# --------------------------------------------- #
# GLEW binaries
# https://sourceforge.net/projects/glew/files/latest/download
#
# GLFW binaries
# https://www.glfw.org/download
#
# --------------------------------------------- #


# --------------------- INCLUDE --------------------- #
include_directories("${PROJECT_SOURCE_DIR}/vendor/glew-2.2.0-win32/include")
include_directories("${PROJECT_SOURCE_DIR}/vendor/glfw-3.3.8.bin.WIN64/include")

# --------------------- SOURCE --------------------- #
add_executable(hyperf_matrix main.cpp tick.cpp
        ComputeShader.cpp
        ComputeShader.h
        glUtils.cpp
        glUtils.h)

# --------------------- LINK --------------------- #
target_link_libraries(hyperf_matrix "${PROJECT_SOURCE_DIR}/vendor/glew-2.2.0-win32/lib/Release/x64/glew32s.lib")
target_link_libraries(hyperf_matrix "${PROJECT_SOURCE_DIR}/vendor/glfw-3.3.8.bin.WIN64/lib-mingw-w64/libglfw3.a")
target_link_libraries(hyperf_matrix opengl32)


# -------------------- MICROS -------------------- #
# add_definitions(-DDEBUG_OPENGL)  # enable opengl debugging
# add_definitions(-DFORCE_USE_DEDICATED_GRAPHICS_CARD)  # force use dedicated graphics card
