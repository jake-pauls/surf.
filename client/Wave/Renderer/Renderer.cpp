#include "Renderer.h"

#include "Vulkan/VkRenderer.h"

core::Unique<wv::Renderer> wv::Renderer::CreateRendererWithGAPI(Window* window, Camera* camera, Renderer::GraphicsAPI gapi)
{
	switch (gapi)
	{
	case Renderer::GraphicsAPI::None:
		WAVE_ASSERT(false, "No rendering API selected");
		return nullptr;
	case Renderer::GraphicsAPI::DirectX:
		WAVE_ASSERT(false, "DirectX is unimplemented");
		return nullptr;
	case Renderer::GraphicsAPI::Vulkan:
		return core::CreateUnique<vkn::VkRenderer>(window, camera);
	};

	WAVE_ASSERT(false, "Unknown rendering API selected");
	return nullptr;
}

