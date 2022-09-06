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
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

/*
void doImgGui(SceneController controller) {
    {
        ImGui::Begin("Window Controls");

        if (ImGui::Button("reload shaders")) {
            controller.reloadShaders();
        }
        if (ImGui::Button("Toggle Ray March")) parameters.renderRayMarch = !parameters.renderRayMarch;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();


        ImGui::Begin("Rendering Controls");

        std::string item_current = render_types[0];            // Here our selection is a single pointer stored outside the object.
        if (ImGui::BeginCombo("Render Type", parameters.active_render_type.c_str())) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < render_types.size(); n++)
            {
                bool is_selected = (parameters.active_render_type == render_types[n]);
                if (ImGui::Selectable(render_types[n].c_str(), is_selected))
                    parameters.active_render_type = render_types[n];
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
            }
            ImGui::EndCombo();
        }
        ImGui::Combo("Shader Output type", &parameters.current_shader_output, shader_output_items, shader_output_count);

        int value = 0;

        ImGui::End();


        ImGui::Begin("Scene Controls");

        ImGui::SliderFloat("Camera Dist Root", &parameters.camera_dist, 0.5f, 30.0f);
        ImGui::SliderFloat("Camera Height", &parameters.camera_height, -10.0f, 10.0f);
        ImGui::SliderFloat2("Camera Rotation", parameters.rotation, 0, glm::two_pi<float>());


        if (ImGui::Button("Toggle Rotate")) parameters.rotateCamera = !parameters.rotateCamera;
        ImGui::End();

        //ImGui::SliderFloat3("Object Rotation", parameters.rotation, 0, glm::two_pi<float>());
        /*
        if (ImGui::Button("Reset Rotation")) {
            parameters.rotation[0] = 0;
            parameters.rotation[1] = 0;
            parameters.rotation[2] = 0;
        }
        */
        

        /*
        ImGui::SliderFloat("Light: Polar Angle", &parameters.light_direction[0], 0, glm::two_pi<float>());
        ImGui::SliderFloat("Light: Azimuth Angle", &parameters.light_direction[1], -glm::half_pi<float>(), glm::half_pi<float>());
        */
        /*
        ImGui::SliderFloat("GGX Parameter", &parameters.ggx_param, 0.1f, 1.0f);
        ImGui::SliderInt("GGX Samples", &parameters.num_ggx_samples, 1, 100);
        
        if (ImGui::Button("Toggle Uniform")) parameters.printUniformNotFound = !parameters.printUniformNotFound;
        ImGui::SameLine();
        if (ImGui::Button("Toggle AABB Outline")) parameters.showAABBOutline = !parameters.showAABBOutline;
        */
        /*
        ImGui::SliderFloat("Ray Marching Delta", &parameters.rayMarchDist, 0.005f, 0.1f);
        ImGui::SliderInt("Ray Marching Max Steps", &parameters.rayMarchMaxSteps, 0.0, 1000);
        
    }
}
*/