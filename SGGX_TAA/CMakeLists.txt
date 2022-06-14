set(PROJECT_NAME SGGX_TAA)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "src/3rd_party/imgui/imconfig.h"
    "src/3rd_party/imgui/imgui.h"
    "src/3rd_party/imgui/imgui_impl_glfw_gl3.h"
    "src/3rd_party/imgui/imgui_internal.h"
    "src/3rd_party/imgui/stb_rect_pack.h"
    "src/3rd_party/imgui/stb_textedit.h"
    "src/3rd_party/imgui/stb_truetype.h"
    "src/3rd_party/obj/tiny_obj_loader.h"
    "src/3rd_party/std_image/std_image.h"
    "src/file_utils.h"
    "src/GL_Utils.h"
    "src/gl_wrappers/ArrayBuffer.h"
    "src/gl_wrappers/IndexBuffer.h"
    "src/gl_wrappers/Shader.h"
    "src/gl_wrappers/Texture.h"
    "src/gl_wrappers/VertexArray.h"
    "src/Parameters.h"
    "src/Renderer/Camera.h"
    "src/Renderer/Light.h"
    "src/Renderer/Material.h"
    "src/Renderer/RayMarchObject.h"
    "src/Renderer/Renderer.h"
    "src/Renderer/SceneController.h"
    "src/Renderer/SceneObject.h"
    "src/Utils.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "src/3rd_party/imgui/imgui.cpp"
    "src/3rd_party/imgui/imgui_draw.cpp"
    "src/3rd_party/imgui/imgui_impl_glfw_gl3.cpp"
    "src/3rd_party/std_image/std_image.cpp"
    "src/Application.cpp"
    "src/gl_wrappers/ArrayBuffer.cpp"
    "src/gl_wrappers/IndexBuffer.cpp"
    "src/gl_wrappers/Shader.cpp"
    "src/gl_wrappers/Texture.cpp"
    "src/gl_wrappers/VertexArray.cpp"
    "src/Parameters.cpp"
    "src/Renderer/Camera.cpp"
    "src/Renderer/Light.cpp"
    "src/Renderer/Material.cpp"
    "src/Renderer/RayMarchObject.cpp"
    "src/Renderer/Renderer.cpp"
    "src/Renderer/SceneController.cpp"
    "src/Renderer/SceneObject.cpp"
    "src/Utils.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
)


add_executable(${PROJECT_NAME} ${ALL_FILES})

################################################################################
# Target
################################################################################
# add_executable(${PROJECT_NAME} ${ALL_FILES})
# 
# use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
# set(ROOT_NAMESPACE SGGXTAA)
# 
# set_target_properties(${PROJECT_NAME} PROPERTIES
#     VS_GLOBAL_KEYWORD "Win32Proj"
# )
# if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
#     set_target_properties(${PROJECT_NAME} PROPERTIES
#         INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
#     )
# elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
#     set_target_properties(${PROJECT_NAME} PROPERTIES
#         INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
#     )
# endif()
# ################################################################################
# # Include directories
# ################################################################################
# if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
#     target_include_directories(${PROJECT_NAME} PUBLIC
#         "${CMAKE_CURRENT_SOURCE_DIR}/home/janluca/Studium/Bachelors/dev/SGGX_TAA/Dependencies/GLFW/include;"
#         "${CMAKE_CURRENT_SOURCE_DIR}/home/janluca/Studium/Bachelors/dev/SGGX_TAA/Dependencies/GLEW/include;"
#         "${CMAKE_CURRENT_SOURCE_DIR}/home/janluca/Studium/Bachelors/dev/SGGX_TAA/Dependencies/glm"
#     )
# endif()
# 
# ################################################################################
# # Compile definitions
# ################################################################################
# if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
#     target_compile_definitions(${PROJECT_NAME} PRIVATE
#         "$<$<CONFIG:Debug>:"
#             "_DEBUG"
#         ">"
#         "$<$<CONFIG:Release>:"
#             "NDEBUG"
#         ">"
#         "_CONSOLE;"
#         "UNICODE;"
#         "_UNICODE"
#     )
# elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
#     target_compile_definitions(${PROJECT_NAME} PRIVATE
#         "$<$<CONFIG:Debug>:"
#             "_DEBUG"
#         ">"
#         "$<$<CONFIG:Release>:"
#             "NDEBUG"
#         ">"
#         "WIN32;"
#         "_CONSOLE;"
#         "GLEW_STATIC;"
#         "UNICODE;"
#         "_UNICODE"
#     )
# endif()
# 
# ################################################################################
# # Compile and link options
# ################################################################################
# if(MSVC)
#     if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
#         target_compile_options(${PROJECT_NAME} PRIVATE
#             $<$<CONFIG:Release>:
#                 /Oi;
#                 /Gy
#             >
#             /permissive-;
#             /sdl;
#             /W3;
#             ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
#             ${DEFAULT_CXX_EXCEPTION_HANDLING}
#         )
#     elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
#         target_compile_options(${PROJECT_NAME} PRIVATE
#             $<$<CONFIG:Release>:
#                 /Oi;
#                 /Gy
#             >
#             /permissive-;
#             /std:c++17;
#             /sdl;
#             /W3;
#             ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
#             ${DEFAULT_CXX_EXCEPTION_HANDLING}
#         )
#     endif()
#     if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
#         target_link_options(${PROJECT_NAME} PRIVATE
#             $<$<CONFIG:Debug>:
#                 /INCREMENTAL
#             >
#             $<$<CONFIG:Release>:
#                 /OPT:REF;
#                 /OPT:ICF;
#                 /INCREMENTAL:NO
#             >
#             /DEBUG;
#             /SUBSYSTEM:CONSOLE
#         )
#     elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
#         target_link_options(${PROJECT_NAME} PRIVATE
#             $<$<CONFIG:Debug>:
#                 /INCREMENTAL
#             >
#             $<$<CONFIG:Release>:
#                 /OPT:REF;
#                 /OPT:ICF;
#                 /INCREMENTAL:NO
#             >
#             /DEBUG;
#             /SUBSYSTEM:CONSOLE
#         )
#     endif()
# endif()
# 
# ################################################################################
# # Dependencies
# ################################################################################
# if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
#     set(ADDITIONAL_LIBRARY_DEPENDENCIES
#         "glfw3;"
#         "opengl32;"
#         "User32;"
#         "Gdi32;"
#         "Shell32;"
#         "glew32s"
#     )
# endif()
# target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
# 
# if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
#     target_link_directories(${PROJECT_NAME} PRIVATE
#         "${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2019;"
#         "${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/Win32"
#     )
# endif()
# 
# 