cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

project(SGGX_TAA)

foreach( d ${COMPILE_DEFINITIONS} )
    message( STATUS "Found Define: " ${d} )
endforeach()



include_directories(${CMAKE_SOURCE_DIR}/Dependencies/glew-2.1.0/include/)
include_directories(${CMAKE_SOURCE_DIR}/Dependencies/glfw-3.3.7/include/)
include_directories(Dependencies/glm-0.9.9.8/)

add_subdirectory(SGGX_TAA)

# if(DEFINED ENV{RENDERDOC_PATH})
#     set(RENDERDOC_PATH ENV{RENDERDOC_PATH})
# elseif(WIN32)
#     set(RENDERDOC_PATH "C:\\Program Files\\RenderDoc")
# endif() # TODO LINUX

### Don't forget that there is a CMakeLists.txt in libs ! ###

################################################################################
# Source groups
################################################################################
#set(Header_Files
#    "src/3rd_party/imgui/imconfig.h"
#    "src/3rd_party/imgui/imgui.h"
#    "src/3rd_party/imgui/imgui_impl_glfw_gl3.h"
#    "src/3rd_party/imgui/imgui_internal.h"
#    "src/3rd_party/imgui/stb_rect_pack.h"
#    "src/3rd_party/imgui/stb_textedit.h"
#    "src/3rd_party/imgui/stb_truetype.h"
#    "src/3rd_party/obj/tiny_obj_loader.h"
#    "src/3rd_party/std_image/std_image.h"
#    "src/file_utils.h"
#    "src/GL_Utils.h"
#    "src/gl_wrappers/ArrayBuffer.h"
#    "src/gl_wrappers/IndexBuffer.h"
#    "src/gl_wrappers/Shader.h"
#    "src/gl_wrappers/Texture.h"
#    "src/gl_wrappers/VertexArray.h"
#    "src/Parameters.h"
#    "src/Renderer/Camera.h"
#    "src/Renderer/Light.h"
#    "src/Renderer/Material.h"
#    "src/Renderer/RayMarchObject.h"
#    "src/Renderer/Renderer.h"
#    "src/Renderer/SceneController.h"
#    "src/Renderer/SceneObject.h"
#    "src/Utils.h"
#)
#source_group("Header Files" FILES ${Header_Files})
#
#set(Source_Files
#    "src/3rd_party/imgui/imgui.cpp"
#    "src/3rd_party/imgui/imgui_draw.cpp"
#    "src/3rd_party/imgui/imgui_impl_glfw_gl3.cpp"
#    "src/3rd_party/std_image/std_image.cpp"
#    "src/Application.cpp"
#    "src/gl_wrappers/ArrayBuffer.cpp"
#    "src/gl_wrappers/IndexBuffer.cpp"
#    "src/gl_wrappers/Shader.cpp"
#    "src/gl_wrappers/Texture.cpp"
#    "src/gl_wrappers/VertexArray.cpp"
#    "src/Parameters.cpp"
#    "src/Renderer/Camera.cpp"
#    "src/Renderer/Light.cpp"
#    "src/Renderer/Material.cpp"
#    "src/Renderer/RayMarchObject.cpp"
#    "src/Renderer/Renderer.cpp"
#    "src/Renderer/SceneController.cpp"
#    "src/Renderer/SceneObject.cpp"
#    "src/Utils.cpp"
#)
#source_group("Source Files" FILES ${Source_Files})
#
#set(ALL_FILES
#    ${Header_Files}
#    ${Source_Files}
#)


# add_shader(${PROJECT_NAME} shaders/sort.comp)
# add_shader(${PROJECT_NAME} shaders/buildOctree.comp)

# add_subdirectory(./vendor/curlpp)
# set_property(TARGET curlpp STATIC IMPORTED ./vendor/curlpp/libcurlpp.a)
# target_link_libraries(my_target curlpp)


# find_package(OpenGL REQUIRED)
# target_link_libraries(${PROJECT_NAME} OpenGL::GL)


# GLEW

set(OpenGL_GL_PREFERENCE GLVND)

add_subdirectory(Dependencies/glew-2.1.0/build/cmake)
target_link_libraries(${PROJECT_NAME} glew)

link_directories(${CMAKE_SOURCE_DIR}/Dependencies/glew-2.1.0/src/)

# GLFW

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/Dependencies/glfw-3.3.7)
target_link_libraries(${PROJECT_NAME} glfw)

link_directories(${CMAKE_SOURCE_DIR}/Dependencies/glfw-3.3.7/src/)

# GLM
# TODO Look up correct way
add_subdirectory(${CMAKE_SOURCE_DIR}/Dependencies/glm-0.9.9.8)
target_link_libraries(${PROJECT_NAME} glm)

link_directories(${CMAKE_SOURCE_DIR}/Dependencies/glm-0.9.9.8/)



#target_include_directories(${PROJECT_NAME} PUBLIC ./include)
# target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS} )
# target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan ${CMAKE_DL_LIBS})
# target_compile_definitions(${PROJECT_NAME} PRIVATE)
