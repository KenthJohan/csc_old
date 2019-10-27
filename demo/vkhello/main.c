#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <string.h>

#include <csc_malloc_file.h>
#include <csc_basic.h>
#include <csc_vk.h>
#include <csc_glfw.h>

#include "misc.h"


void run()
{
	VkSurfaceKHR surface;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	glfwInit ();
	glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow (WIDTH, HEIGHT, "Vulkan", NULL, NULL);
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
	if (glfwCreateWindowSurface (instance, window, NULL, &surface) != VK_SUCCESS)
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
		vkDestroySemaphore (dev.logical, renderFinishedSemaphores[i], NULL);
		vkDestroySemaphore (dev.logical, imageAvailableSemaphores[i], NULL);
		vkDestroyFence (dev.logical, inFlightFences[i], NULL);
	}
	vkDestroyCommandPool (dev.logical, commandPool, NULL);
	for (uint32_t i = 0; i < swapchain.count; i++)
	{
		vkDestroyFramebuffer (dev.logical, swapchain.framebuffers [i], NULL);
	}
	vkDestroyPipeline (dev.logical, pipeline.graphicsPipeline, NULL);
	vkDestroyPipelineLayout (dev.logical, pipeline.pipelineLayout, NULL);
	vkDestroyRenderPass (dev.logical, renderpass.renderpass, NULL);
	for (uint32_t i = 0; i < swapchain.count; i++)
	{
		vkDestroyImageView (dev.logical, swapchain.imageviews [i], NULL);
	}
	vkDestroySwapchainKHR (dev.logical, swapchain.swapchain, NULL);
	vkDestroyDevice (dev.logical, NULL);
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
	}
	vkDestroySurfaceKHR (instance, surface, NULL);
	vkDestroyInstance (instance, NULL);
	glfwDestroyWindow (window);
	glfwTerminate ();
}


int main()
{
	run ();
	return EXIT_SUCCESS;
}
