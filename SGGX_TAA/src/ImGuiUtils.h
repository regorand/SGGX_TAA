#pragma once

#include <vector>
#include <string>

#include "3rd_party/imgui/imgui.h"
//#include "3rd_party/imgui/imgui_impl_glfw_gl3.h"

#include "Renderer/SceneController.h"
#include "Types.h"
#include "file_utils.h"

void doCombo(const std::string title, std::string& selection, std::vector<std::string>& content) {
    std::string item_current = content[0];            // Here our selection is a single pointer stored outside the object.
    if (ImGui::BeginCombo(title.c_str(), selection.c_str())) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < content.size(); n++)
        {
            bool is_selected = (selection == content[n]);
            if (ImGui::Selectable(content[n].c_str(), is_selected))
                selection = content[n];
            if (is_selected)
                ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        }
        ImGui::EndCombo();
    }
}

void doCombo(const std::string title, std::string& selection, int &selection_index, std::vector<std::string>& content) {
    std::string item_current = content[0];            // Here our selection is a single pointer stored outside the object.
    if (ImGui::BeginCombo(title.c_str(), selection.c_str())) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < content.size(); n++)
        {
            bool is_selected = (selection == content[n]);
            if (ImGui::Selectable(content[n].c_str(), is_selected)) {
                selection = content[n];
                selection_index = n;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        }
        ImGui::EndCombo();
    }
}

void initImGui() {
    updateLoadableObjects();
}

void doImGui(SceneController &controller) {
    ImGui::Begin("Window Controls");

    if (ImGui::Button("reload shaders")) {
        controller.reloadShaders();
    }
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();


    ImGui::Begin("Rendering Controls");

    doCombo("Render Type", parameters.active_render_type, render_types);
    doCombo("Shader Output Type", parameters.active_shader_output, parameters.active_shader_output_index, shader_output_types);

    ImGui::NewLine();

    if (loadableObjs.size() > 0) {
        doCombo("Loadable Obj", parameters.selected_file, loadableObjs);
    }
    if (ImGui::Button("Load Selected File")) {
        if (parameters.selected_file != "") {
            controller.loadAndDisplayObject(parameters.selected_file);
        }
    }

    //ImGui::Combo("Shader Output type", &parameters.current_shader_output, shader_output_items, shader_output_count);

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
    */
}
