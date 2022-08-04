cmake_minimum_required(VERSION 3.22.0)
project(EdgeUI VERSION 0.1.0)
include(CPack)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

# pkg-config --cflags gtk4
# pkg-config --cflags cairo
include_directories(
    include
    ${GTK4_INCLUDE_DIRS}
)

file(GLOB SOURCE_FILES source/*.cpp)

set( COMMON_SOURCE_FILES
    source/GL/FreeTypeFont.cpp
    source/GL/GLDiagnostics.cpp
    source/GL/GLShader.cpp
    source/GL/Graphics_GL.cpp
)
#-DVERBOSE_SHADER_BUILD
#-DVERBOSE_BUILD
#*************** X11 Target
add_library(EdgeUI.X11 ${SOURCE_FILES})
set_property(TARGET EdgeUI.X11 PROPERTY CXX_STANDARD 17)
target_compile_definitions(EdgeUI.X11 PUBLIC)
target_sources(EdgeUI.X11 PUBLIC
    ${COMMON_SOURCE_FILES}
    source/GL/PlatformInterface_X11.cpp
)
if( CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(EdgeUI.X11 PUBLIC)
endif( CMAKE_BUILD_TYPE STREQUAL Debug)

#*************** GTK4 Target
add_library(EdgeUI.GTK4 ${SOURCE_FILES})
set_property(TARGET EdgeUI.X11 PROPERTY CXX_STANDARD 17)
target_compile_definitions(EdgeUI.GTK4 PUBLIC)
target_sources(EdgeUI.GTK4 PUBLIC
    ${COMMON_SOURCE_FILES}
    source/GL/PlatformInterface_GTK4.cpp
)
if( CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(EdgeUI.GTK4 PUBLIC)
endif( CMAKE_BUILD_TYPE STREQUAL Debug)


#*************** Direct render manager (DRM) Target
add_library(EdgeUI.DRM ${SOURCE_FILES})
set_property(TARGET EdgeUI.DRM PROPERTY CXX_STANDARD 17)
target_compile_definitions(EdgeUI.DRM PUBLIC)
target_sources(EdgeUI.DRM PUBLIC
    ${COMMON_SOURCE_FILES}
    source/GL/PlatformInterface_DRM.cpp
)
target_include_directories(EdgeUI.DRM PUBLIC /usr/include/libdrm)
if( CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(EdgeUI.DRM PUBLIC)
endif( CMAKE_BUILD_TYPE STREQUAL Debug)




