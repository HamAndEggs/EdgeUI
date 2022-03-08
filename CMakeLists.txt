cmake_minimum_required(VERSION 3.0.0)
project(EdgeUI VERSION 0.1.0)

include(CTest)
enable_testing()


add_definitions(-DPLATFORM_X11_GL -DVERBOSE_BUILD -DVERBOSE_SHADER_BUILD)

include_directories(
    /usr/include/freetype2
    /usr/include/libdrm
    source
)

file(GLOB SOURCE_FILES
    source/*.cpp
    source/GL/*.cpp
)
add_library(EdgeUI ${SOURCE_FILES})
set_property(TARGET EdgeUI PROPERTY CXX_STANDARD 17)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
