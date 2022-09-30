#pragma once

#include <vector>
#include <string>

#include <chrono>


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

	render_type& current = render_types[parameters.current_render_type_index];
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
	ImGui::SameLine();
	ImGui::Checkbox("Do Camera Path", &camera_params.doCameraPath);
	if (ImGui::Button("Export Frame")) {
		auto millisec_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (millisec_since_epoch - parameters.last_image_export > 500) {
			parameters.last_image_export = millisec_since_epoch;
			controller.exportImage(millisec_since_epoch);
		}
	}
	ImGui::SameLine();
	ImGui::Checkbox("Export Video", &parameters.writeVideo);

	ImGui::NewLine();
	ImGui::Checkbox("Flat Shade", &parameters.flat_shade);
	ImGui::SliderInt("Voxel Count", &parameters.voxel_count, 10, 300);
	ImGui::SliderFloat("SGGX Roughness", &octree_params.roughness, 0, 1);
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

	if (camera_params.doCameraPath) {
		ImGui::Begin("Camera Path Controls");
		ImGui::SliderInt("Current Frame", &camera_params.current_frame, camera_params.min_frame, camera_params.max_frame);
		ImGui::Checkbox("Auto play", &camera_params.autoplay);
		ImGui::SameLine();
		ImGui::Checkbox("Lock Camera to path", &camera_params.lock_to_path);
		if (ImGui::Button("Save Keyframe")) {
			controller.addKeyframe();
		}
		ImGui::SameLine();
		ImGui::InputInt("New Frame", &camera_params.add_frame);
		if (ImGui::Button("Reset Keyframes")) {
			controller.resetKeyframes();
		}
		ImGui::End();
	}

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

		//ImGui::SliderInt("Min Render Depth", &octree_params.min_render_depth, 0, octree_params.max_render_depth - 1);
		//ImGui::SliderInt("Max Render Depth", &octree_params.max_render_depth, octree_params.min_render_depth + 1, 16);
		// Level of detail given by how deep we render the tree
		ImGui::SliderFloat("Level of Detail", &octree_params.render_depth, 0, 16.0f);

		ImGui::SliderInt("Number Iterations", &octree_params.num_iterations, 1, 128);

		ImGui::SliderInt("Roentgen Denom", &octree_params.roentgen_denominator, 1, 100);
		ImGui::Checkbox("Auto LOD", &octree_params.auto_lod);
		ImGui::SameLine();
		ImGui::Checkbox("smooth LOD", &octree_params.smooth_lod);
		ImGui::Checkbox("Interpolate voxels", &taa_params.interpolate_voxels);
		ImGui::SliderFloat("Diffuse Parameter", &parameters.diffuse_parameter, 0, 1);

		ImGui::End();


		// TAA Controls
		ImGui::Begin("TAA Controls");
		ImGui::SliderFloat("Alpha", &taa_params.alpha, 0.0, 1.0);
		ImGui::Checkbox("Enable TAA", &taa_params.taa_active);
		ImGui::SameLine();
		ImGui::Checkbox("Vis active alpha", &taa_params.visualize_active_alpha);
		ImGui::Spacing();
		ImGui::Checkbox("Disable Jiggle", &taa_params.disable_jiggle);
		ImGui::Checkbox("reprojection", &taa_params.do_reprojection);
		ImGui::SameLine();
		ImGui::Checkbox("Vis motion vectors", &taa_params.visualize_motion_vectors);

		ImGui::SliderFloat("Slider Jiggle", &taa_params.jiggle_factor, 0, 5);
		ImGui::Spacing();
		ImGui::Checkbox("Enable History Rejection", &taa_params.doHistoryRejection);
		ImGui::Checkbox("Visualize History Rejection", &taa_params.visualizeHistoryRejection);
		ImGui::SameLine();
		ImGui::Checkbox("Vis Egde Detection", &taa_params.visualize_edge_detection);
		ImGui::SliderInt("History Rejection Buffer Depth", &taa_params.historyRejectionBufferDepth, 1, 4);
		ImGui::Checkbox("Interpolate alpha", &taa_params.interpolate_alpha);
		ImGui::SliderInt("History Rejection Parent Level", &taa_params.historyParentRejectionLevel, 0, 10);

		ImGui::Spacing();
		ImGui::Checkbox("Apply offset", &taa_params.apply_lod_offset);
		ImGui::SameLine();
		ImGui::Checkbox("Visualize LoD Offset", &taa_params.visualize_feedback_level);
		std::string current_feedback_level = LoD_feedback_types[taa_params.Lod_feedback_level];
		doCombo("LoD Feedback Type", current_feedback_level, taa_params.Lod_feedback_level, LoD_feedback_types);
		ImGui::SliderInt("Max LoD Diff", &taa_params.max_lod_diff, 0, (int)octree_params.render_depth);

		ImGui::End();
	}
}
