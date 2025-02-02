#include "ToolPanel.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl3.h>
#include <glm/glm.hpp>

#include "SurfEngine.h"
#include "SurfStore.h"

namespace wv
{
	std::string ToolPanel::s_SelectedModel = "Sphere";
	std::string ToolPanel::s_SelectedMaterial = "Default";
}

wv::ToolPanel::ToolPanel(surf_Gapi gapi)
{
	switch (gapi)
	{
	case SURF_GAPI_NIL:
		WAVE_ASSERT(false, "No graphics API selected");
		break;
	case SURF_GAPI_DIRECTX:
		WAVE_ASSERT(false, "DirectX is unimplemented");
		break;
	case SURF_GAPI_OPENGL:
		WAVE_ASSERT(false, "OpenGL is unimplemented");
		break;
	case SURF_GAPI_VULKAN:
		m_ImGuiNewFrameFunction = &ImGui_ImplVulkan_NewFrame;
		break;
	}
}

void wv::ToolPanel::Draw()
{
	m_ImGuiNewFrameFunction();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("wave. tool panel");
	ImGui::SetWindowSize(ImVec2(350.0f, 230.0f), ImGuiCond_FirstUseEver);
	ImGui::SetWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_FirstUseEver);

	if (ImGui::CollapsingHeader("Render Settings"))
	{
		ImGui::Text("Model Settings");

		const float width = ImGui::GetWindowWidth();
		const float comboWidth = width * 0.5f;

		// Note: 'VkRenderer' matches against these strings when selecting new meshes
		const char* models[] = { "Sphere", "Viking Room", "Rock", "Teapot",  "Bunny", "Suzanne", "Dragon" };
		static const char* currentItem = models[0];
		ImGui::SetNextItemWidth(comboWidth);
		if (ImGui::BeginCombo("Model", currentItem))
		{
			for (size_t i = 0; i < IM_ARRAYSIZE(models); ++i)
			{
				bool isSelected = currentItem == models[i];
				if (ImGui::Selectable(models[i], isSelected))
				{
					currentItem = models[i];
					s_SelectedModel = currentItem;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		const char* materials[] = { "Default", "Viking Room", "PBR Default", "Rock (PBR)", "Rust (PBR)", "Bamboo (PBR)", "Sand (PBR)"};
		static const char* currentMaterial = materials[0];
		ImGui::SetNextItemWidth(comboWidth);
		if (ImGui::BeginCombo("Material", currentMaterial))
		{
			for (size_t i = 0; i < IM_ARRAYSIZE(materials); ++i)
			{
				bool isSelected = currentMaterial == materials[i];
				if (ImGui::Selectable(materials[i], isSelected))
				{
					currentMaterial = materials[i];
					s_SelectedMaterial = currentMaterial;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	if (ImGui::CollapsingHeader("Uniform Data", ImGuiTreeNodeFlags_None))
	{
		glm::vec3 albedo = SurfStore::s_SurfAlbedo;
		float metallic = SurfStore::s_SurfMetallic;
		float roughness = SurfStore::s_SurfRoughness;
		float ao = SurfStore::s_SurfAO;
		glm::vec3 lightPosition = SurfStore::s_SurfLightPosition;
		glm::vec3 lightColor = SurfStore::s_SurfLightColor;

		ImGui::Text("pbr.surf");
		ImGui::BulletText("");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(albedo.x, albedo.y, albedo.z, 1.0f), "pbr_albedo:");
		ImGui::SameLine();
		ImGui::Text("(%.2f, %.2f, %.2f)", albedo.x, albedo.y, albedo.z);
		ImGui::BulletText("pbr_metallic: %.2f", metallic);
		ImGui::BulletText("pbr_roughness: %.2f", roughness);
		ImGui::BulletText("pbr_ao: %.2f", ao);
		ImGui::BulletText("pbr_light_position: (%.2f, %.2f, %.2f)", lightPosition.x, lightPosition.y, lightPosition.z);
		ImGui::BulletText("");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(lightColor.x / 255, lightColor.y / 255, lightColor.z / 255, 1.0f), "pbr_light_color:");
		ImGui::SameLine();
		ImGui::Text("(%.2f, %.2f, %.2f)", lightColor.x, lightColor.y, lightColor.z);
	}

	ImGui::End();
}
