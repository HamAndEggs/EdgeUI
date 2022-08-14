cmake_minimum_required(VERSION 3.18.0)
project(EdgeUI VERSION 0.1.0)
include(CPack)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include_directories(
    include
    /usr/include/freetype2/
)

file(GLOB SOURCE_FILES source/*.cpp)

set( COMMON_SOURCE_FILES
    source/GL/FreeTypeFont.cpp
    source/GL/GLDiagnostics.cpp
    source/GL/GLShader.cpp
    source/GL/Graphics_GL.cpp
)

set( COMMON_DEBUG_COMPILE_OPTIONS
    -Wall
    -Wpedantic
    -Werror
    -Wno-unused-function
    -Wno-unused-variable)

set( COMMON_DEBUG_DEFINES
    -DVERBOSE_BUILD
    -DVERBOSE_SHADER_BUILD)

#*************** X11 Target
add_library(EdgeUI.X11 ${SOURCE_FILES})
set_property(TARGET EdgeUI.X11 PROPERTY CXX_STANDARD 17)
target_compile_definitions(EdgeUI.X11 PUBLIC)
target_sources(EdgeUI.X11 PRIVATE
    ${COMMON_SOURCE_FILES}
    source/GL/PlatformInterface_X11.cpp
)
if( CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(EdgeUI.X11 PUBLIC ${COMMON_DEBUG_DEFINES})
    target_compile_options(EdgeUI.X11 PUBLIC ${COMMON_DEBUG_COMPILE_OPTIONS})
endif( CMAKE_BUILD_TYPE STREQUAL Debug)

#*************** GTK4 Target
add_library(EdgeUI.GTK4 ${SOURCE_FILES})
set_property(TARGET EdgeUI.X11 PROPERTY CXX_STANDARD 17)
target_compile_definitions(EdgeUI.GTK4 PUBLIC -DPLATFORM_GLES)
target_sources(EdgeUI.GTK4 PRIVATE
    ${COMMON_SOURCE_FILES}
    source/GL/PlatformInterface_GTK4.cpp
)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 gtk4)

# pkg-config --cflags gtk4
# pkg-config --cflags cairo

target_include_directories(EdgeUI.GTK4 PUBLIC ${GTK4_INCLUDE_DIRS})

if( CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(EdgeUI.GTK4 PUBLIC ${COMMON_DEBUG_DEFINES})
    target_compile_options(EdgeUI.GTK4 PUBLIC ${COMMON_DEBUG_COMPILE_OPTIONS})
endif( CMAKE_BUILD_TYPE STREQUAL Debug)


#*************** Direct render manager (DRM) Target
add_library(EdgeUI.DRM ${SOURCE_FILES})
set_property(TARGET EdgeUI.DRM PROPERTY CXX_STANDARD 17)
target_compile_definitions(EdgeUI.DRM PUBLIC -DPLATFORM_GLES)
target_sources(EdgeUI.DRM PRIVATE
    ${COMMON_SOURCE_FILES}
    source/GL/PlatformInterface_DRM.cpp
)
target_include_directories(EdgeUI.DRM PUBLIC /usr/include/libdrm)
if( CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(EdgeUI.DRM PUBLIC ${COMMON_DEBUG_DEFINES})
    target_compile_options(EdgeUI.DRM PUBLIC ${COMMON_DEBUG_COMPILE_OPTIONS})
endif( CMAKE_BUILD_TYPE STREQUAL Debug)




