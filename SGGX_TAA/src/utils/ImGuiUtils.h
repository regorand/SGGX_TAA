#pragma once

#include <vector>
#include <string>

#include "../3rd_party/imgui/imgui.h"
//#include "3rd_party/imgui/imgui_impl_glfw_gl3.h"

#include "../Renderer/SceneController.h"
#include "../Types.h"
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

void doCombo(const std::string title, std::string& selection, int& selection_index, std::vector<std::string>& content) {
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

void doImGui(SceneController& controller) {
	ImGui::Begin("Window Controls");

	if (ImGui::Button("reload shaders")) {
		controller.reloadShaders();
	}
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();


	ImGui::Begin("Rendering Controls");


	render_type &current = render_types[parameters.current_render_type_index];
	if (parameters.current_render_type_index == parameters.old_render_type_index) {
		current.current_render_type = parameters.current_shader_output_index;
	}
	else {
		parameters.old_render_type_index = parameters.current_render_type_index;
		parameters.current_shader_output_index = current.current_render_type;
	}
	std::string shader_output_name = current.shader_output_types[current.current_render_type];
	doCombo("Render Type", current.name, parameters.current_render_type_index, render_types_names);
	doCombo("Shader Output Type", shader_output_name, parameters.current_shader_output_index, current.shader_output_types);

	//doCombo("Render Type", parameters.active_render_type, render_types_2);
	//doCombo("Shader Output Type OLD", parameters.active_shader_output, parameters.active_shader_output_index, shader_output_types);
	ImGui::Checkbox("Render Voxels AABB", &parameters.renderVoxelsAABB);

	ImGui::NewLine();

	ImGui::SliderInt("Voxel Count", &parameters.voxel_count, 10, 300);
	if (loadableObjs.size() > 0) {
		doCombo("Loadable Obj", parameters.selected_file, loadableObjs);
	}
	if (ImGui::Button("Load Selected File")) {
		if (parameters.selected_file != "") {
			controller.loadAndDisplayObject(parameters.selected_file);
		}
	}
	ImGui::SameLine();
	ImGui::Checkbox("Force reload", &parameters.forceReload);

	//ImGui::Combo("Shader Output type", &parameters.current_shader_output, shader_output_items, shader_output_count);

	ImGui::End();

	float eps = 1e-3;

	ImGui::Begin("Scene Controls");

	ImGui::SliderFloat("Camera Dist Root", &camera_params.camera_dist, 0.5f, 30.0f);
	ImGui::SliderFloat3("Look At", camera_params.lookAtPos, -10.0f, 10.0f);
	ImGui::SameLine();
	if (ImGui::Button("reset look at")) {
		camera_params.lookAtPos[0] = 0;
		camera_params.lookAtPos[1] = 0;
		camera_params.lookAtPos[2] = 0;
	}
	ImGui::SameLine();
	if (ImGui::Button("look at object center")) {
		controller.lookAtObject();
	}

	ImGui::SliderFloat("Azitmuth Angle", &camera_params.cameraPos[0], 0, glm::two_pi<float>());
	ImGui::SameLine();
	ImGui::Checkbox("Rotate Azimuth", &camera_params.rotateAzimuth);
	ImGui::SameLine();
	if (ImGui::Button("reset Azimuth")) {
		camera_params.cameraPos[0] = 0;
		camera_params.rotateAzimuth = false;
	}

	ImGui::SliderFloat("Polar Angle", &camera_params.cameraPos[1], -glm::pi<float>(), glm::pi<float>()); ImGui::SameLine();
	ImGui::Checkbox("Rotate Polar", &camera_params.rotatePolar);
	ImGui::SameLine();
	if (ImGui::Button("reset polar")) {
		camera_params.cameraPos[1] = 0;
		camera_params.rotatePolar = false;
	}

	ImGui::SliderFloat("FOV", &camera_params.fov, glm::pi<float>() / 6, glm::pi<float>());
	ImGui::SameLine();
	if (ImGui::Button("reset fov")) {
		camera_params.fov = glm::pi<float>() / 3;
	}
	ImGui::End();

	// Probably can combine this OCTREE_RENDER_INDEX
	if (parameters.current_render_type_index == OCTREE_VIS_RENDER_INDEX)
	{
		ImGui::Begin("Octree Visualization Controls");
		ImGui::SliderInt("Max Tree Depth", &octree_params.max_tree_depth, 0, 16);

		ImGui::SliderInt("Min Visualization Depth", &parameters.min_visualization_depth, 0, parameters.max_visualization_depth - 1);
		ImGui::SliderInt("Max Visualization Depth", &parameters.max_visualization_depth, parameters.min_visualization_depth + 1, 16);
		if (ImGui::Button("Reload Octree Visualization")) {
			controller.reloadOctreeVis();
		}
		ImGui::End();
	}
	else if (parameters.current_render_type_index == OCTREE_RENDER_INDEX) {
		ImGui::Begin("Octree Controls");
		ImGui::SliderInt("Max Tree Depth", &octree_params.max_tree_depth, 0, 16);

		ImGui::SliderInt("Min Render Depth", &octree_params.min_render_depth, 0, octree_params.max_render_depth - 1);
		ImGui::SliderInt("Max Render Depth", &octree_params.max_render_depth, octree_params.min_render_depth + 1, 16);

		ImGui::SliderInt("Number Iterations", &octree_params.num_iterations, 1, 128);

		ImGui::SliderInt("Roentgen Denom", &octree_params.roentgen_denominator, 1, 100);
		ImGui::Checkbox("Auto LOD", &octree_params.auto_lod);
		ImGui::Checkbox("New Tree Building", &octree_params.new_building);

		ImGui::End();
	}

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
