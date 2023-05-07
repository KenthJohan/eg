#include "renderer.h"


#include <vector>
#include <array>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <set>
#include <unordered_map>


#include "readfile.h"
#include "load_model.h"
#include "render1.h"
#include "render2.h"
#include "create_image.h"

#include "types.hpp"
#include "updateUniformBuffer.hpp"

#include "EgVk.h"
#include "EgVkPhysicalDevices.h"
#include "EgVkLayers.h"
#include "EgVkInstanceExtensions.h"
#include "EgVkDeviceExtensions.h"
#include "EgVkQueueFamilies.h"
#include "EgVkInstances.h"
#include "EgVkDevices.h"

#include "EgTypes.h"
#include "EgWindows.h"
#include "EgGeometries.h"
#include "EgPlatform.h"
#include "EgLogs.h"

#include "eg_util.h"
#include "eg_basics.h"






#define WIDTH 800
#define HEIGHT 600

#define MODEL_PATH "viking_room.obj"

#define FRAMEBUFFER_ATTACHMENT_COUNT 3
#define RENDERPASS_CLEARVALUE_COUNT 2
#define VALIDATION_LAYER_COUNT 1
#define DEVICE_EXTENSION_COUNT 1

char const * const validationLayers[VALIDATION_LAYER_COUNT] = {
"VK_LAYER_KHRONOS_validation"
};

char const * const deviceExtensions[DEVICE_EXTENSION_COUNT] = {
VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif






static VkVertexInputBindingDescription getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

	return attributeDescriptions;
}





class HelloTriangleApplication {
public:

	ecs_world_t* world;
	ecs_entity_t entity_instance;

	render2_context_t context;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	//VkSwapchainKHR context.swapchain;
	//std::vector<VkImage> swapChainImages;
	//VkFormat swapChainImageFormat;
	//VkExtent2D swapChainExtent;
	//uint32_t context.swapChainImageViews_count;
	//VkImageView context.swapchain_imageview[32];
	//VkFramebuffer context.swapchain_framebuffer[32];

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VkCommandPool commandPool;



	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
	VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];

	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
	VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
	VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
	uint32_t currentFrame = 0;

	bool framebufferResized = false;


	void initVulkan()
	{
		render1_createRenderPass(world, context.physical, context.device, msaaSamples, VK_FORMAT_B8G8R8A8_SRGB, &renderPass);

		EgRectangleI32 * r = (EgRectangleI32 *)ecs_get(world, entity_instance, EgRectangleI32);
		createSwapChain(&context.swapchain, world, context.physical, context.device, context.surface, r->width, r->height, msaaSamples, renderPass);

		createDescriptorSetLayout(world, context.device, &descriptorSetLayout);
		createGraphicsPipeline();
		createCommandPool();
		createTextureImage(world, context.physical, context.device, commandPool, context.qf_graphics, &textureImage, &mipLevels, &textureImageMemory);
		createTextureImageView();
		createTextureSampler();
		loadModel(vertices, indices, MODEL_PATH);
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool(world, context.device, &descriptorPool);
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}





	void cleanup()
	{
		swapchain_cleanup(context.device, &context.swapchain);

		vkDestroyPipeline(context.device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(context.device, pipelineLayout, nullptr);
		vkDestroyRenderPass(context.device, renderPass, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(context.device, uniformBuffers[i], nullptr);
			vkFreeMemory(context.device, uniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(context.device, descriptorPool, nullptr);

		vkDestroySampler(context.device, textureSampler, nullptr);
		vkDestroyImageView(context.device, textureImageView, nullptr);

		vkDestroyImage(context.device, textureImage, nullptr);
		vkFreeMemory(context.device, textureImageMemory, nullptr);

		vkDestroyDescriptorSetLayout(context.device, descriptorSetLayout, nullptr);

		vkDestroyBuffer(context.device, indexBuffer, nullptr);
		vkFreeMemory(context.device, indexBufferMemory, nullptr);

		vkDestroyBuffer(context.device, vertexBuffer, nullptr);
		vkFreeMemory(context.device, vertexBufferMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(context.device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(context.device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(context.device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(context.device, commandPool, nullptr);

		vkDestroyDevice(context.device, nullptr);

		//vkDestroySurfaceKHR(instance, surface, nullptr);
		//vkDestroyInstance(instance, nullptr);

		//glfwDestroyWindow(window);
		//glfwTerminate();
	}

	void recreateSwapChain()
	{
		EG_EVENT_STRF(world, EgLogsVerbose, "recreateSwapChain\n");
		eg_platform_wait_positive_framebuffer_size(world, entity_instance);

		//To wait on the host for the completion of outstanding queue operations for all queues on a given logical device
		vkDeviceWaitIdle(context.device);

		swapchain_cleanup(context.device, &context.swapchain);
		EgRectangleI32 * r = (EgRectangleI32 *)ecs_get(world, entity_instance, EgRectangleI32);
		createSwapChain(&context.swapchain, world, context.physical, context.device, context.surface, r->width, r->height, msaaSamples, renderPass);
	}





	void createGraphicsPipeline()
	{
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
		{
			long length = 0;
			char * code = readfile1("vert.spv", &length);
			vertShaderModule = createShaderModule(code, length);
		}
		{
			long length = 0;
			char * code = readfile1("frag.spv", &length);
			fragShaderModule = createShaderModule(code, length);
		}

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescription = getBindingDescription();
		auto attributeDescriptions = getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = msaaSamples;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();



		{
			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 1;
			pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
			VkResult result = vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
			if (result != VK_SUCCESS)
			{
				EG_EVENT_STRF(world, EgVkLogError, "vkCreatePipelineLayout failed");
			}
		}


		{
			VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			pipelineInfo.layout = pipelineLayout;
			pipelineInfo.renderPass = renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			VkResult result = vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
			if (result != VK_SUCCESS)
			{
				EG_EVENT_STRF(world, EgVkLogError, "vkCreateGraphicsPipelines failed");
			}
		}

		vkDestroyShaderModule(context.device, fragShaderModule, nullptr);
		vkDestroyShaderModule(context.device, vertShaderModule, nullptr);
	}


	void createCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = context.qf_index_graphics;
		VkResult result = vkCreateCommandPool(context.device, &poolInfo, nullptr, &commandPool);
		if (result != VK_SUCCESS)
		{
			EG_EVENT_STRF(world, EgVkLogError, "vkCreateCommandPool failed");
		}
	}





	bool hasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}







	void createTextureImageView()
	{
		textureImageView = createImageView(world, context.device, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	}

	void createTextureSampler()
	{
		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(context.physical, &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(mipLevels);
		samplerInfo.mipLodBias = 0.0f;

		VkResult result = vkCreateSampler(context.device, &samplerInfo, nullptr, &textureSampler);
		if (result != VK_SUCCESS)
		{
			EG_EVENT_STRF(world, EgVkLogError, "vkCreateSampler failed");
		}
	}











	void createVertexBuffer() {
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(world, context.physical, context.device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

		void* data;
		vkMapMemory(context.device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t) bufferSize);
		vkUnmapMemory(context.device, stagingBufferMemory);

		createBuffer(world, context.physical, context.device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

		copyBuffer(context.device, context.qf_graphics, commandPool, stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(context.device, stagingBuffer, nullptr);
		vkFreeMemory(context.device, stagingBufferMemory, nullptr);
	}

	void createIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(world, context.physical, context.device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

		void* data;
		vkMapMemory(context.device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t) bufferSize);
		vkUnmapMemory(context.device, stagingBufferMemory);

		createBuffer(world, context.physical, context.device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);

		copyBuffer(context.device, context.qf_graphics, commandPool, stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(context.device, stagingBuffer, nullptr);
		vkFreeMemory(context.device, stagingBufferMemory, nullptr);
	}

	void createUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			createBuffer(world, context.physical, context.device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniformBuffers[i], &uniformBuffersMemory[i]);
		}
	}



	void createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		VkResult result = vkAllocateDescriptorSets(context.device, &allocInfo, descriptorSets);
		if (result != VK_SUCCESS)
		{
			EG_EVENT_STRF(world, EgVkLogError, "vkAllocateDescriptorSets failed");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(context.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}









	void createCommandBuffers() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t) MAX_FRAMES_IN_FLIGHT;

		VkResult result = vkAllocateCommandBuffers(context.device, &allocInfo, commandBuffers);
		if (result != VK_SUCCESS)
		{
			EG_EVENT_STRF(world, EgVkLogError, "vkAllocateCommandBuffers failed");
		}
	}

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
			if (result != VK_SUCCESS)
			{
				EG_EVENT_STRF(world, EgVkLogError, "vkBeginCommandBuffer failed");
			}
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = context.swapchain.swapchain_framebuffer[imageIndex];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = context.swapchain.swapchain_create_info.imageExtent;


		VkClearValue clearValues[RENDERPASS_CLEARVALUE_COUNT] = {
		{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
		{.depthStencil = {1.0f, 0}}
		};

		renderPassInfo.clearValueCount = RENDERPASS_CLEARVALUE_COUNT;
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float) context.swapchain.swapchain_create_info.imageExtent.width;
		viewport.height = (float) context.swapchain.swapchain_create_info.imageExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = context.swapchain.swapchain_create_info.imageExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkBuffer vertexBuffers[] = {vertexBuffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		VkResult result = vkEndCommandBuffer(commandBuffer);
		if (result != VK_SUCCESS)
		{
			EG_EVENT_STRF(world, EgVkLogError, "vkEndCommandBuffer failed");
		}
	}

	void createSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(context.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			{
				EG_EVENT_STRF(world, EgVkLogError, "failed to create synchronization objects for a frame!");
			}
		}
	}



	void drawFrame()
	{
		vkWaitForFences(context.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(context.device, context.swapchain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			EG_EVENT_STRF(world, EgVkLogError, "failed to acquire swap chain image!");
		}

		updateUniformBuffer(context.device, context.swapchain.swapchain_create_info.imageExtent, uniformBuffersMemory[currentFrame]);

		vkResetFences(context.device, 1, &inFlightFences[currentFrame]);

		vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

		VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		{
			VkResult result = vkQueueSubmit(context.qf_graphics, 1, &submitInfo, inFlightFences[currentFrame]);
			if (result != VK_SUCCESS)
			{
				EG_EVENT_STRF(world, EgVkLogError, "vkQueueSubmit failed");
			}
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = {context.swapchain.swapchain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(context.qf_present, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			EG_EVENT_STRF(world, EgVkLogError, "failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	VkShaderModule createShaderModule(char const * code, size_t size)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = size;
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

		VkShaderModule shaderModule;

		VkResult result = vkCreateShaderModule(context.device, &createInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
		{
			EG_EVENT_STRF(world, EgVkLogError, "vkCreateShaderModule failed");
		}

		return shaderModule;
	}




};


HelloTriangleApplication app;



void renderer_init()
{


	app.world = ecs_init();
	ECS_IMPORT(app.world, EgLogs);
	ECS_IMPORT(app.world, EgWindows);
	ECS_IMPORT(app.world, EgGeometries);
	ECS_IMPORT(app.world, EgVk);
	ECS_IMPORT(app.world, EgVkInstances);
	ECS_IMPORT(app.world, EgPlatform);
	ECS_IMPORT(app.world, EgTypes);
	ECS_IMPORT(app.world, EgVkLayers);
	ECS_IMPORT(app.world, EgVkQueueFamilies);
	ECS_IMPORT(app.world, EgVkInstanceExtensions);
	ECS_IMPORT(app.world, EgVkDeviceExtensions);
	ECS_IMPORT(app.world, EgVkPhysicalDevices);
	ECS_IMPORT(app.world, EgVkDevices);

	try
	{
		app.entity_instance = render1_init(app.world);
		app.context.physical = render1_pick_physical_device(app.world);
		app.context.surface = ecs_get(app.world, app.entity_instance, EgVkSurfaceKHR)->surface;
		app.msaaSamples = render1_get_max_usable_sample_count(app.context.physical);
		render2_config(&app.context);

		app.initVulkan();
	}
	catch (const std::exception& e)
	{
		EG_EVENT_STRF(app.world, EgLogsError, "std::exception: %s\n", e.what());
	}
}

void renderer_fini()
{
	app.cleanup();
	ecs_fini(app.world);
}

int renderer_update()
{
	ecs_progress(app.world, 0);
	eg_platform_update();
	app.drawFrame();
	vkDeviceWaitIdle(app.context.device);
	//int r = glfwWindowShouldClose(app.window);
	//return r;
}