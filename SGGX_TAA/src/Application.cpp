#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <iostream>
#include <memory>


#include "3rd_party/imgui/imgui.h"
#include "3rd_party/imgui/imgui_impl_glfw_gl3.h"

#include "utils/GL_Utils.h"

#include "Renderer/SceneController.h"
#include "Parameters.h"
#include "Types.h"

#include "utils/ImGuiUtils.h"

int main(void)
{
    const std::string vert_path = "res/shaders/shader.vert";
    const std::string frag_path = "res/shaders/shader.frag";

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    /* Create a windowed mode window and its OpenGL context */
    //window = glfwCreateWindow(1280, 960, "Hello World", NULL, NULL);

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    int xpos, ypos;
    glfwGetMonitorWorkarea(primary, &xpos, &ypos, &parameters.windowWidth, &parameters.windowHeight);

    window = glfwCreateWindow(parameters.windowWidth, parameters.windowHeight, "SGGX TAA", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMaximizeWindow(window);
    
    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);

    ImGui::StyleColorsDark();

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "Error" << std::endl;
        return 1;
    }

    registerGLCallBack();

    SceneController controller;
    if (!controller.init()) {
        std::cout << "Could not init SceneController" << std::endl;
        return 1;
    }

    glEnable(GL_DEPTH_TEST);

    initParams();
    initImGui();


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplGlfwGL3_NewFrame();

        doImGui(controller);

        controller.doFrame();

        ImGui::Render();
        glFinish();

        glfwSwapInterval(parameters.vsync);

        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
        glFinish();
    }

    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}