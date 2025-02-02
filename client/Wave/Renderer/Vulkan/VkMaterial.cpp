#include "VkMaterial.h"

#if defined(__linux__)
#define STBI_NO_SIMD
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "VkRenderer.h"

vkn::VkMaterial::VkMaterial(const VkRenderer& renderer, const VkShaderPipeline& shaderPipeline) 
	: c_VkRenderer(&renderer)
	, c_VkHardware(renderer.m_VkHardware)
	, m_ShaderPipeline(&shaderPipeline) 
	, m_IsTexturedMaterial(false)
{ 
}

vkn::VkMaterial::VkMaterial(const VkRenderer& renderer, const VkShaderPipeline& shaderPipeline, const std::vector<std::string>& textureNames)
	: c_VkRenderer(&renderer)
	, c_VkHardware(renderer.m_VkHardware)
	, m_ShaderPipeline(&shaderPipeline)
	, m_IsTexturedMaterial(true)
{
	m_Textures.resize(textureNames.size());

	for (size_t i = 0; i < textureNames.size(); ++i)
	{ 
		std::string texturePath = core::FileSystem::GetAssetPath(textureNames[i].c_str()).string();
		CreateTexture(texturePath, &m_Textures[i]);

		VkImageViewCreateInfo textureInfo = vkn::InitImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, m_Textures[i].m_Image.m_Image, VK_IMAGE_ASPECT_COLOR_BIT);
		vkCreateImageView(c_VkHardware->m_LogicalDevice, &textureInfo, nullptr, &m_Textures[i].m_Image.m_ImageView);
	}

	AllocateSamplerDescriptorSet();
}

vkn::VkMaterial::~VkMaterial()
{
	if (m_IsTexturedMaterial)
	{
		for (size_t i = 0; i < m_Textures.size(); ++i)
		{
			vkDestroySampler(c_VkHardware->m_LogicalDevice, m_Textures[i].m_Sampler, nullptr);
			vkDestroyImageView(c_VkHardware->m_LogicalDevice, m_Textures[i].m_Image.m_ImageView, nullptr);
			vmaDestroyImage(c_VkHardware->m_VmaAllocator, m_Textures[i].m_Image.m_Image, m_Textures[i].m_Image.m_Allocation);
		}
	}
}

void vkn::VkMaterial::BindMaterialTextures(VkCommandBuffer commandBuffer) const
{
	if (!m_IsTexturedMaterial)
		return;

	vkCmdBindDescriptorSets(commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_ShaderPipeline->m_PipelineLayout, 1, 1,
		&m_TextureDescriptorSet, 0, nullptr);
}

void vkn::VkMaterial::AllocateSamplerDescriptorSet()
{
	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		m_Textures[i].m_Sampler = VkSampler();
		VkSamplerCreateInfo samplerCreateInfo = vkn::InitSamplerCreateInfo(VK_FILTER_NEAREST);
		vkCreateSampler(c_VkHardware->m_LogicalDevice, &samplerCreateInfo, nullptr, &m_Textures[i].m_Sampler);
	}

	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = vkn::InitDescriptorSetAllocateInfo(m_ShaderPipeline->m_DescriptorPool, &m_ShaderPipeline->m_TextureSetLayout);
	VK_CALL(vkAllocateDescriptorSets(c_VkHardware->m_LogicalDevice, &descriptorSetAllocInfo, &m_TextureDescriptorSet));

	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		VkDescriptorImageInfo textureBufferInfo = VkDescriptorImageInfo();
		textureBufferInfo.sampler = m_Textures[i].m_Sampler;
		textureBufferInfo.imageView = m_Textures[i].m_Image.m_ImageView;
		textureBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet descriptorSetWrite = vkn::InitWriteDescriptorSetImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_TextureDescriptorSet, &textureBufferInfo, static_cast<uint32_t>(i));
		writeDescriptorSets.push_back(descriptorSetWrite);
	}

	vkUpdateDescriptorSets(c_VkHardware->m_LogicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void vkn::VkMaterial::CreateTexture(const std::string& filename, VkTexture* outTexture)
{
	int texWidth, texHeight, channels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
	WAVE_ASSERT(pixels, "Failed to load texture from provided image file");

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	VmaAllocatedBuffer stagingBuffer;
	c_VkHardware->CreateVMABuffer(stagingBuffer,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_ONLY);

	void* texData;
	VK_CALL(vmaMapMemory(c_VkHardware->m_VmaAllocator, stagingBuffer.m_Allocation, &texData));
	memcpy(texData, pixels, static_cast<size_t>(imageSize));
	vmaUnmapMemory(c_VkHardware->m_VmaAllocator, stagingBuffer.m_Allocation);

	stbi_image_free(pixels);

	/// ----------------

	VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

	VkExtent3D imageExtent;
	imageExtent.width = static_cast<uint32_t>(texWidth);
	imageExtent.height = static_cast<uint32_t>(texHeight);
	imageExtent.depth = 1;
	
	VkImageCreateInfo newImageCreateInfo = vkn::InitImageCreateInfo(imageFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);
	newImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	VmaAllocationCreateInfo newImageAllocationCreateInfo = VmaAllocationCreateInfo();
	newImageAllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	// Allocate and create image
	vmaCreateImage(c_VkHardware->m_VmaAllocator, &newImageCreateInfo, &newImageAllocationCreateInfo, &outTexture->m_Image.m_Image, &outTexture->m_Image.m_Allocation, nullptr);

	/// ----------------

	// Submit command to render image
	c_VkRenderer->SubmitToRenderer([=, this](VkCommandBuffer commandBuffer) 
	{
		VkImageSubresourceRange range = VkImageSubresourceRange();
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkImageMemoryBarrier transferImageBarrier = VkImageMemoryBarrier();
		transferImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		transferImageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		transferImageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		transferImageBarrier.image = outTexture->m_Image.m_Image;
		transferImageBarrier.subresourceRange = range;
		transferImageBarrier.srcAccessMask = 0;
		transferImageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		// Barrier the image into the transfer-receive layout
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &transferImageBarrier);

		VkBufferImageCopy copyRegion = VkBufferImageCopy();
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = imageExtent;

		// Copy the image into a staging buffer
		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.m_Buffer, outTexture->m_Image.m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		VkImageMemoryBarrier readableImageBarrier = transferImageBarrier;
		readableImageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		readableImageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		readableImageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		readableImageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		// Barrier the image into the shader layout
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &readableImageBarrier);
	});

	/// ----------------

	// Cleanup local resources
	vmaDestroyBuffer(c_VkHardware->m_VmaAllocator, stagingBuffer.m_Buffer, stagingBuffer.m_Allocation);
}
