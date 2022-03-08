cmake_minimum_required(VERSION 3.0.0)
project(HelloWorld VERSION 0.1.0)

include(CTest)
enable_testing()


add_definitions(-DPLATFORM_X11_GL -DVERBOSE_BUILD -DVERBOSE_SHADER_BUILD)

include_directories(
    /usr/include/freetype2
    /usr/include/libdrm
    ../../source
)

add_subdirectory(/home/richard/development/EdgeUI)

add_executable(HelloWorld HelloWorld.cpp)

target_link_libraries(HelloWorld EdgeUI)

set_property(TARGET HelloWorld PROPERTY CXX_STANDARD 17)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
