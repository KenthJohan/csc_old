#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <set>

#include <csc_malloc_file.h>
#include <csc_basic.h>
#include <csc_vk.h>
#include <csc_glfw.h>

#define WIDTH 800
#define HEIGHT 600
#define MAX_FRAMES_IN_FLIGHT 2

#define VALIDATON_LAYER_COUNT 1
static char const * validationLayers [VALIDATON_LAYER_COUNT] =
{
"VK_LAYER_KHRONOS_validation"
//"VK_LAYER_LUNARG_standard_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT
(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL)
	{
		return func (instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT
(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL)
	{
		func (instance, debugMessenger, pAllocator);
	}
}

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


static size_t currentFrame = 0;


void cleanup (struct csc_vk_device * dev, struct csc_vk_swapchain * swapchain, struct csc_vk_pipeline * pipeline, struct csc_vk_renderpass * renderpass, VkCommandPool commandpool)
{

	vkDestroyCommandPool (dev->logical, commandpool, nullptr);

	for (uint32_t i = 0; i < swapchain->count; i++)
	{
		vkDestroyFramebuffer (dev->logical, swapchain->framebuffers [i], nullptr);
	}

	vkDestroyPipeline (dev->logical, pipeline->graphicsPipeline, nullptr);
	vkDestroyPipelineLayout (dev->logical, pipeline->pipelineLayout, nullptr);
	vkDestroyRenderPass (dev->logical, renderpass->renderpass, nullptr);

	for (uint32_t i = 0; i < swapchain->count; i++)
	{
		vkDestroyImageView (dev->logical, swapchain->imageviews [i], nullptr);
	}

	vkDestroySwapchainKHR (dev->logical, swapchain->swapchain, nullptr);
	vkDestroyDevice (dev->logical, nullptr);
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	(VkDebugUtilsMessageTypeFlagsEXT)messageType;
	(VkDebugUtilsMessengerCallbackDataEXT*)pUserData;
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {fprintf (stderr, "VERBOSE|");}
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {fprintf (stderr, "VERBOSE|");}
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {fprintf (stderr, "VERBOSE|");}
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {fprintf (stderr, "VERBOSE");}
	fprintf (stderr, "\n");
	fprintf (stderr, "validation layer: %s\n", pCallbackData->pMessage);
	return VK_FALSE;
}


void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void setupDebugMessenger (VkInstance instance, VkDebugUtilsMessengerEXT * debugMessenger)
{
	if (!enableValidationLayers) {return;}
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo (createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, debugMessenger) != VK_SUCCESS)
	{
		perror ("failed to set up debug messenger!");
		exit (EXIT_FAILURE);
	}
}



void createInstance (VkInstance * instance)
{
	if (enableValidationLayers && !csc_vk_layers_exist (validationLayers, VALIDATON_LAYER_COUNT))
	{
		perror ("validation layers requested, but not available!");
		exit (EXIT_FAILURE);
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	char const * ext [3];
	ext [0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	createInfo.enabledExtensionCount = 1;
	createInfo.enabledExtensionCount += csc_glfw_copy_required_instance_extensions (ext + 1, 2);
	createInfo.ppEnabledExtensionNames = ext;
	csc_vk_pinfo (&createInfo);

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = VALIDATON_LAYER_COUNT;
		createInfo.ppEnabledLayerNames = validationLayers;

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}
	if (vkCreateInstance(&createInfo, nullptr, instance) != VK_SUCCESS)
	{
		perror ("failed to create instance!");
		exit (EXIT_FAILURE);
	}
}

SwapChainSupportDetails querySwapChainSupport (VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

void pickPhysicalDevice (VkInstance instance, struct csc_vk_device * dev, VkSurfaceKHR surface)
{
	VkPhysicalDevice phys [10];
	uint32_t n = 10;
	uint32_t format_count = 0;
	uint32_t presentmodes_count = 0;
	vkEnumeratePhysicalDevices (instance, &n, NULL);
	vkEnumeratePhysicalDevices (instance, &n, phys);
	for (uint32_t i = 0; i < n; i ++)
	{
		dev->family_gfx = UINT32_MAX;
		dev->family_present = UINT32_MAX;
		dev->family_transfer = UINT32_MAX;
		vkGetPhysicalDeviceSurfaceFormatsKHR (phys[i], surface, &format_count, NULL);
		vkGetPhysicalDeviceSurfacePresentModesKHR (phys[i], surface, &presentmodes_count, NULL);
		csc_vk_find_famqueue3 (phys [i], surface, &dev->family_gfx, &dev->family_present, &dev->family_transfer);
		if (dev->family_gfx != UINT32_MAX && dev->family_present != UINT32_MAX)
		{
			dev->phys = phys [i];
			break;
		}
	}
	if (dev->phys == VK_NULL_HANDLE)
	{
		perror ("failed to find a suitable GPU!");
		exit (EXIT_FAILURE);
	}
}

void createLogicalDevice (struct csc_vk_device * dev)
{
	uint32_t n = 1;
	float priority = 1.0f;
	VkDeviceQueueCreateInfo q [3];
	q[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	q[0].pNext = NULL;
	q[0].flags = 0;
	q[0].queueFamilyIndex = dev->family_gfx;
	q[0].queueCount = 1;
	q[0].pQueuePriorities = &priority;
	q[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	q[1].pNext = NULL;
	q[1].flags = 0;
	q[1].queueFamilyIndex = 0;
	q[1].queueCount = 1;
	q[1].pQueuePriorities = &priority;
	q[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	q[2].pNext = NULL;
	q[2].flags = 0;
	q[2].queueFamilyIndex = 0;
	q[2].queueCount = 1;
	q[2].pQueuePriorities = &priority;
	if (dev->family_present != dev->family_gfx)
	{
		q[n].queueFamilyIndex = dev->family_present;
		n++;
	}
	if (dev->family_transfer != dev->family_gfx && dev->family_transfer != dev->family_present)
	{
		q[n].queueFamilyIndex = dev->family_transfer;
		n++;
	}
	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	info.queueCreateInfoCount = n;
	info.pQueueCreateInfos = q;
	info.pEnabledFeatures = &deviceFeatures;
	char const * ext[] =
	{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	info.enabledExtensionCount = countof (ext);
	info.ppEnabledExtensionNames = ext;
	if (enableValidationLayers)
	{
		info.enabledLayerCount = VALIDATON_LAYER_COUNT;
		info.ppEnabledLayerNames = validationLayers;
	}
	else
	{
		info.enabledLayerCount = 0;
		info.ppEnabledLayerNames = NULL;
	}
	if (vkCreateDevice (dev->phys, &info, nullptr, &dev->logical) != VK_SUCCESS)
	{
		perror ("failed to create logical device!");
		exit (EXIT_FAILURE);
	}
	vkGetDeviceQueue (dev->logical, dev->family_gfx, 0, &dev->graphicsQueue);
	vkGetDeviceQueue (dev->logical, dev->family_present, 0, &dev->presentQueue);
}


void createImageViews (VkDevice logical, struct csc_vk_swapchain * swapchain)
{
	for (uint32_t i = 0; i < swapchain->count; i++)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchain->images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchain->format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView (logical, &createInfo, nullptr, &swapchain->imageviews[i]) != VK_SUCCESS)
		{
			perror ("failed to create image views!");
			exit (EXIT_FAILURE);
		}
	}
}

void createRenderPass (struct csc_vk_device * dev, struct csc_vk_swapchain * swapchain, struct csc_vk_renderpass * renderpass)
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchain->format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass (dev->logical, &renderPassInfo, nullptr, &renderpass->renderpass) != VK_SUCCESS)
	{
		perror ("failed to create render pass!");
		exit (EXIT_FAILURE);
	}
}


VkShaderModule createShaderModule (struct csc_vk_device * dev, char const * filename)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	long length = 0;
	char const * code = csc_malloc_file1 (filename, &length);
	ASSERT (length > 0)
	createInfo.codeSize = (size_t) length;
	//Any memory that's allocated dynamically via new or malloc is guaranteed to be properly aligned for objects of any type
	createInfo.pCode = (uint32_t const*) code;
	VkShaderModule shaderModule;
	if (vkCreateShaderModule (dev->logical, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		perror ("failed to create shader module!");
		exit (EXIT_FAILURE);
	}
	return shaderModule;
}


void createGraphicsPipeline (struct csc_vk_device * dev, struct csc_vk_swapchain * swapchain, struct csc_vk_pipeline * pipeline, struct csc_vk_renderpass * renderpass)
{
	VkShaderModule vertShaderModule = createShaderModule (dev, "../vkhello/shader.vert.spv");
	VkShaderModule fragShaderModule = createShaderModule (dev, "../vkhello/shader.frag.spv");

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapchain->extent.width;
	viewport.height = (float) swapchain->extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapchain->extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout (dev->logical, &pipelineLayoutInfo, nullptr, &pipeline->pipelineLayout) != VK_SUCCESS)
	{
		perror ("failed to create pipeline layout!");
		exit (EXIT_FAILURE);
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipeline->pipelineLayout;
	pipelineInfo.renderPass = renderpass->renderpass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines (dev->logical, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline->graphicsPipeline) != VK_SUCCESS)
	{
		perror ("failed to create graphics pipeline!");
		exit (EXIT_FAILURE);
	}

	vkDestroyShaderModule (dev->logical, fragShaderModule, nullptr);
	vkDestroyShaderModule (dev->logical, vertShaderModule, nullptr);
}

void createFramebuffers (struct csc_vk_device * dev, struct csc_vk_swapchain * swapchain, struct csc_vk_renderpass * renderpass)
{
	//swapChainFramebuffers.resize (swapchain->count);
	for (size_t i = 0; i < swapchain->count; i++)
	{
		VkImageView attachments[] =
		{
		swapchain->imageviews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderpass->renderpass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapchain->extent.width;
		framebufferInfo.height = swapchain->extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer (dev->logical, &framebufferInfo, nullptr, &swapchain->framebuffers[i]) != VK_SUCCESS)
		{
			perror ("failed to create framebuffer!");
			exit (EXIT_FAILURE);
		}
	}
}

void createCommandPool (struct csc_vk_device * dev, VkCommandPool * commandpool)
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = dev->family_gfx;
	if (vkCreateCommandPool (dev->logical, &poolInfo, nullptr, commandpool) != VK_SUCCESS)
	{
		perror ("failed to create command pool!");
		exit (EXIT_FAILURE);
	}
}

void createCommandBuffers (struct csc_vk_device * dev, struct csc_vk_swapchain * swapchain, struct csc_vk_pipeline * pipeline, struct csc_vk_renderpass * renderpass, VkCommandPool commandpool)
{
	//commandBuffers.resize (swapchain->count);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandpool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) swapchain->count;

	if (vkAllocateCommandBuffers (dev->logical, &allocInfo, swapchain->commandbuffers) != VK_SUCCESS)
	{
		perror ("failed to allocate command buffers!");
		exit (EXIT_FAILURE);
	}

	for (size_t i = 0; i < swapchain->count; i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(swapchain->commandbuffers [i], &beginInfo) != VK_SUCCESS)
		{
			perror ("failed to begin recording command buffer!");
			exit (EXIT_FAILURE);
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderpass->renderpass;
		renderPassInfo.framebuffer = swapchain->framebuffers [i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapchain->extent;

		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass (swapchain->commandbuffers [i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline (swapchain->commandbuffers [i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->graphicsPipeline);

		vkCmdDraw (swapchain->commandbuffers [i], 3, 1, 0, 0);

		vkCmdEndRenderPass (swapchain->commandbuffers [i]);

		if (vkEndCommandBuffer (swapchain->commandbuffers [i]) != VK_SUCCESS)
		{
			perror("failed to record command buffer!");
			exit (EXIT_FAILURE);
		}
	}
}

void createSyncObjects (struct csc_vk_device * dev, VkSemaphore imageAvailableSemaphores[], VkSemaphore renderFinishedSemaphores[], VkFence inFlightFences[])
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore (dev->logical, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
		vkCreateSemaphore (dev->logical, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
		vkCreateFence (dev->logical, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			perror ("failed to create synchronization objects for a frame!");
			exit (EXIT_FAILURE);
		}
	}
}

void drawFrame (struct csc_vk_device * dev, struct csc_vk_swapchain * swapchain, VkSemaphore imageAvailableSemaphores[], VkSemaphore renderFinishedSemaphores [], VkFence inFlightFences[])
{
	vkWaitForFences (dev->logical, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences (dev->logical, 1, &inFlightFences[currentFrame]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR (dev->logical, swapchain->swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &swapchain->commandbuffers [imageIndex];

	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit (dev->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		perror ("failed to submit draw command buffer!");
		exit (EXIT_FAILURE);
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {swapchain->swapchain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR (dev->presentQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode (const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent (const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = {WIDTH, HEIGHT};
		actualExtent.width  = CLAMP (actualExtent.width,  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width);
		actualExtent.height = CLAMP (actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return actualExtent;
	}
}







void createSwapChain (struct csc_vk_device * dev, VkSurfaceKHR surface, struct csc_vk_swapchain * swapchain)
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport (dev->phys, surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	csc_vk_pinfo_qf (dev->phys, surface);

	uint32_t queueFamilyIndices [2] = {UINT32_MAX};
	csc_vk_find_famqueue3 (dev->phys, surface, queueFamilyIndices + 0, queueFamilyIndices + 1, NULL);
	ASSERT (queueFamilyIndices [0] != UINT32_MAX)
	ASSERT (queueFamilyIndices [1] != UINT32_MAX)
	TRACE_F("%i %i", queueFamilyIndices [0], queueFamilyIndices [1]);

	if (queueFamilyIndices [0] != queueFamilyIndices [1])
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR (dev->logical, &createInfo, nullptr, &swapchain->swapchain) != VK_SUCCESS)
	{
		perror ("failed to create swap chain!");
		exit (EXIT_FAILURE);
	}

	vkGetSwapchainImagesKHR (dev->logical, swapchain->swapchain, &swapchain->count, nullptr);
	swapchain->images = (VkImage*)malloc (sizeof (VkImage) * swapchain->count);
	swapchain->imageviews = (VkImageView*)malloc (sizeof (VkImageView) * swapchain->count);
	swapchain->framebuffers = (VkFramebuffer*)malloc (sizeof (VkFramebuffer) * swapchain->count);
	swapchain->commandbuffers = (VkCommandBuffer*)malloc (sizeof (VkCommandBuffer) * swapchain->count);
	vkGetSwapchainImagesKHR (dev->logical, swapchain->swapchain, &swapchain->count, swapchain->images);

	swapchain->format = surfaceFormat.format;
	swapchain->extent = extent;
}


void run()
{
	VkSurfaceKHR surface;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	glfwInit ();
	glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow (WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	struct csc_vk_device dev;
	struct csc_vk_swapchain swapchain;
	struct csc_vk_pipeline pipeline;
	struct csc_vk_renderpass renderpass;
	VkSemaphore imageAvailableSemaphores [MAX_FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphores [MAX_FRAMES_IN_FLIGHT];
	VkFence inFlightFences [MAX_FRAMES_IN_FLIGHT];
	VkCommandPool commandPool;
	createInstance (&instance);
	setupDebugMessenger (instance, &debugMessenger);
	if (glfwCreateWindowSurface (instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		perror ("failed to create window surface!");
		exit (EXIT_FAILURE);
	}
	pickPhysicalDevice (instance, &dev, surface);
	createLogicalDevice (&dev);
	createSwapChain (&dev, surface, &swapchain);
	createImageViews (dev.logical, &swapchain);
	createRenderPass (&dev, &swapchain, &renderpass);
	createGraphicsPipeline (&dev, &swapchain, &pipeline, &renderpass);
	createFramebuffers (&dev, &swapchain, &renderpass);
	createCommandPool (&dev, &commandPool);
	createCommandBuffers (&dev, &swapchain, &pipeline, &renderpass, commandPool);
	createSyncObjects (&dev, imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences);
	while (!glfwWindowShouldClose (window))
	{
		glfwPollEvents ();
		drawFrame (&dev, &swapchain, imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences);
	}
	vkDeviceWaitIdle (dev.logical);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore (dev.logical, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore (dev.logical, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence (dev.logical, inFlightFences[i], nullptr);
	}
	cleanup (&dev, &swapchain, &pipeline, &renderpass, commandPool);
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	vkDestroySurfaceKHR (instance, surface, nullptr);
	vkDestroyInstance (instance, nullptr);
	glfwDestroyWindow (window);
	glfwTerminate ();
}


int main()
{
	run ();
	return EXIT_SUCCESS;
}
