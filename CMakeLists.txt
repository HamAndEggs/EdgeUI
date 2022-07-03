cmake_minimum_required(VERSION 3.22.0)
project(EdgeUI VERSION 0.1.0)
include(CPack)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})


include_directories(
    /usr/include/freetype2
    include
)


file(GLOB SOURCE_FILES source/*.cpp)

#*************** X11 Target
add_library(EdgeUI.X11 ${SOURCE_FILES})
set_property(TARGET EdgeUI.X11 PROPERTY CXX_STANDARD 17)
target_compile_definitions(EdgeUI.X11 PUBLIC -DPLATFORM_X11_GL)
target_sources(EdgeUI.X11 PUBLIC
    source/GL/FreeTypeFont.cpp
    source/GL/GLDiagnostics.cpp
    source/GL/GLShader.cpp
    source/GL/Graphics_GL.cpp
    source/GL/PlatformInterface_X11.cpp
)
if( CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(EdgeUI.X11 PUBLIC -DVERBOSE_BUILD -DVERBOSE_SHADER_BUILD)
endif( CMAKE_BUILD_TYPE STREQUAL Debug)

#*************** Wayland Target
add_library(EdgeUI.Wayland ${SOURCE_FILES})
set_property(TARGET EdgeUI.Wayland PROPERTY CXX_STANDARD 17)
target_compile_definitions(EdgeUI.Wayland PUBLIC -DPLATFORM_WAYLAND_GL)
target_sources(EdgeUI.Wayland PUBLIC
    source/GL/FreeTypeFont.cpp
    source/GL/GLDiagnostics.cpp
    source/GL/GLShader.cpp
    source/GL/Graphics_GL.cpp
    source/GL/PlatformInterface_Wayland.cpp
)
if( CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(EdgeUI.X11 PUBLIC -DVERBOSE_BUILD -DVERBOSE_SHADER_BUILD)
endif( CMAKE_BUILD_TYPE STREQUAL Debug)


#*************** Direct render manager (DRM) Target
add_library(EdgeUI.DRM ${SOURCE_FILES})
set_property(TARGET EdgeUI.DRM PROPERTY CXX_STANDARD 17)
target_compile_definitions(EdgeUI.DRM PUBLIC -DPLATFORM_DRM_EGL)
target_sources(EdgeUI.DRM PUBLIC
    source/GL/FreeTypeFont.cpp
    source/GL/GLDiagnostics.cpp
    source/GL/GLShader.cpp
    source/GL/Graphics_GL.cpp
    source/GL/PlatformInterface_DRM.cpp
)
target_include_directories(EdgeUI.DRM PUBLIC /usr/include/libdrm)
if( CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(EdgeUI.DRM PUBLIC -DVERBOSE_BUILD -DVERBOSE_SHADER_BUILD)
endif( CMAKE_BUILD_TYPE STREQUAL Debug)




