cmake_minimum_required(VERSION 3.28)
project(arcaie VERSION 1.0.0 LANGUAGES CXX)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# remove boms and correct encoding to make modules compiled properly
set(CMAKE_CXX_SOURCE_FILE_ENCODING UTF-8 NO-BOM)
# enable module features
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dacfd01daf")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
# tell vscode where the modules are
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
else()
    add_compile_options(-O2)
endif()

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# cpp-only srcs
file(GLOB_RECURSE SOURCES ${SRC_DIR}/*.cpp)

# search for modules and module impls
file(GLOB_RECURSE MODULE_IFACES CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/module/*.cppm")

file(GLOB_RECURSE MODULE_IMPLS  CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/module/*.cpp")

# add executable
add_executable(arcaie ${SOURCES})

# add modules and module impls
target_sources(arcaie
    PUBLIC FILE_SET CXX_MODULES FILES ${MODULE_IFACES}
    PRIVATE ${MODULE_IMPLS} src/main.cpp)

# compatibility for includes
target_include_directories(arcaie PRIVATE ${INCLUDE_DIR})

set(MSYS2_ROOT "C:/msys64/mingw64")

# link msys2 packages
target_include_directories(arcaie PRIVATE
    ${MSYS2_ROOT}/include
    ${MSYS2_ROOT}/include/freetype2
)

target_link_directories(arcaie PRIVATE
    ${MSYS2_ROOT}/lib
    ${CMAKE_SOURCE_DIR}/lib
)

# dynamic libs.
target_link_libraries(arcaie PRIVATE
    openal
    freetype
    fmt
    glfw3
    glew32
    brotlienc
    brotlidec
    brotlicommon
    lua
    user32
    kernel32
    opengl32
    ws2_32
    mswsock
    wsock32
)

# win32 special flags
if(WIN32)
    target_link_options(arcaie PRIVATE
        # set application type to windows
        -mwindows
        -Wl,--disable-dynamicbase
        -Wl,--image-base,0x140000000
        -Wl,-rpath,dlls
    )
endif()