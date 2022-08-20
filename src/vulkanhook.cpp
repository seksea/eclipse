#include "hooks.hpp"
#include "chook.hpp"
#include "util/log.hpp"
#include "menu/menu.hpp"
//#include "menu/fonts.hpp"

#include <dlfcn.h>

#include <unordered_map>
#include <memory>

// https://vulkan.lunarg.com/
#include <vulkan/vulkan.h>

#include "menu/imgui/imgui.h"
#include "menu/imgui/imgui_freetype.h"
#include "menu/imgui/imgui_impl_sdl.h"
#include "menu/imgui/imgui_impl_vulkan.h"

//#define LOG printf

static VkAllocationCallbacks* 	g_Allocator = NULL;
static VkInstance               g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_FakeDevice = VK_NULL_HANDLE, g_Device = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily = (uint32_t)-1;
static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;
static uint32_t                 g_MinImageCount = 2;
static VkRenderPass				g_RenderPass = VK_NULL_HANDLE;
static ImGui_ImplVulkanH_Frame  g_Frames[8] = {};
static VkExtent2D				g_ImageExtent = {};

static void CleanupDeviceVulkan( );
static void CleanupRenderTarget( );
static void RenderImGui_Vulkan(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);

static bool CreateDeviceVK( ) {
	// Create Vulkan Instance
	{
		VkInstanceCreateInfo create_info = {};
		constexpr const char* instance_extension = "VK_KHR_surface";

		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.enabledExtensionCount = 1;
		create_info.ppEnabledExtensionNames = &instance_extension;

		// Create Vulkan Instance without any debug feature
		vkCreateInstance(&create_info, g_Allocator, &g_Instance);
		LOG("[+] Vulkan: g_Instance: 0x%p\n", g_Instance);
	}

	// Select GPU
	{
		uint32_t gpu_count;
		vkEnumeratePhysicalDevices(g_Instance, &gpu_count, NULL);
		IM_ASSERT(gpu_count > 0);

		VkPhysicalDevice* gpus = new VkPhysicalDevice[sizeof(VkPhysicalDevice) * gpu_count];
		vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus);

		// If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
		// most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
		// dedicated GPUs) is out of scope of this sample.
		int use_gpu = 0;
		for (int i = 0; i < (int)gpu_count; ++i) {
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(gpus[i], &properties);
			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				use_gpu = i;
				break;
			}
		}

		g_PhysicalDevice = gpus[use_gpu];
		LOG("[+] Vulkan: g_PhysicalDevice: 0x%p\n", g_PhysicalDevice);

		delete[ ] gpus;
	}

	// Select graphics queue family
	{
		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, NULL);
		VkQueueFamilyProperties* queues = new VkQueueFamilyProperties[sizeof(VkQueueFamilyProperties) * count];
		vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
		for (uint32_t i = 0; i < count; ++i)
			if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				g_QueueFamily = i;
				break;
			}
		delete[ ] queues;
		IM_ASSERT(g_QueueFamily != (uint32_t)-1);

		LOG("[+] Vulkan: g_QueueFamily: %u\n", g_QueueFamily);
	}

	// Create Logical Device (with 1 queue)
	{
		constexpr const char* device_extension = "VK_KHR_swapchain";
		constexpr const float queue_priority = 1.0f;

		VkDeviceQueueCreateInfo queue_info = {};
		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info.queueFamilyIndex = g_QueueFamily;
		queue_info.queueCount = 1;
		queue_info.pQueuePriorities = &queue_priority;

		VkDeviceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = 1;
		create_info.pQueueCreateInfos = &queue_info;
		create_info.enabledExtensionCount = 1;
		create_info.ppEnabledExtensionNames = &device_extension;

		vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_FakeDevice);

		LOG("[+] Vulkan: g_FakeDevice: 0x%p\n", g_FakeDevice);
	}

	return true;
}

static void CreateRenderTarget(VkDevice device, VkSwapchainKHR swapchain) {
	uint32_t uImageCount;
	vkGetSwapchainImagesKHR(device, swapchain, &uImageCount, NULL);

	VkImage backbuffers[8] = {};
	vkGetSwapchainImagesKHR(device, swapchain, &uImageCount, backbuffers);

	for (uint32_t i = 0; i < uImageCount; ++i) {
		g_Frames[i].Backbuffer = backbuffers[i];

		ImGui_ImplVulkanH_Frame* fd = &g_Frames[i];
		{
			VkCommandPoolCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			info.queueFamilyIndex = g_QueueFamily;

			vkCreateCommandPool(device, &info, g_Allocator, &fd->CommandPool);
		}
		{
			VkCommandBufferAllocateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			info.commandPool = fd->CommandPool;
			info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			info.commandBufferCount = 1;

			vkAllocateCommandBuffers(device, &info, &fd->CommandBuffer);
		}
	}

	// Create the Render Pass
	{
		VkAttachmentDescription attachment = {};
		attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;

		vkCreateRenderPass(device, &info, g_Allocator, &g_RenderPass);
	}

	// Create The Image Views
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = VK_FORMAT_B8G8R8A8_UNORM;

		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;

		for (uint32_t i = 0; i < uImageCount; ++i) {
			ImGui_ImplVulkanH_Frame* fd = &g_Frames[i];
			info.image = fd->Backbuffer;

			vkCreateImageView(device, &info, g_Allocator, &fd->BackbufferView);
		}
	}

	// Create Framebuffer
	{
		VkImageView attachment[1];
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = g_RenderPass;
		info.attachmentCount = 1;
		info.pAttachments = attachment;
		info.layers = 1;

		for (uint32_t i = 0; i < uImageCount; ++i) {
			ImGui_ImplVulkanH_Frame* fd = &g_Frames[i];
			attachment[0] = fd->BackbufferView;

			vkCreateFramebuffer(device, &info, g_Allocator, &fd->Framebuffer);
		}
	}

	if (!g_DescriptorPool) // Create Descriptor Pool.
	{
		constexpr VkDescriptorPoolSize pool_sizes[ ] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		vkCreateDescriptorPool(device, &pool_info, g_Allocator, &g_DescriptorPool);
	}
}

static CHook<VkResult VKAPI_CALL(VkDevice, VkSwapchainKHR, uint64_t, VkSemaphore, VkFence, uint32_t*)> oAcquireNextImageKHR;
static VkResult VKAPI_CALL hkAcquireNextImageKHR(VkDevice device,
												 VkSwapchainKHR swapchain,
												 uint64_t timeout,
												 VkSemaphore semaphore,
												 VkFence fence,
												 uint32_t* pImageIndex) {
	g_Device = device;

	return oAcquireNextImageKHR.originalFn(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

static CHook<VkResult VKAPI_CALL(VkQueue, const VkPresentInfoKHR*)> oQueuePresentKHR;
static VkResult VKAPI_CALL hkQueuePresentKHR(VkQueue queue,
											 const VkPresentInfoKHR* pPresentInfo) {
	RenderImGui_Vulkan(queue, pPresentInfo);

	return oQueuePresentKHR.originalFn(queue, pPresentInfo);
}

static CHook<VkResult VKAPI_CALL(VkDevice, const VkSwapchainCreateInfoKHR*, const VkAllocationCallbacks*, VkSwapchainKHR*)> oCreateSwapchainKHR;
static VkResult VKAPI_CALL hkCreateSwapchainKHR(VkDevice device,
												const VkSwapchainCreateInfoKHR* pCreateInfo,
												const VkAllocationCallbacks* pAllocator,
												VkSwapchainKHR* pSwapchain) {
	CleanupRenderTarget( );
	g_ImageExtent = pCreateInfo->imageExtent;

	return oCreateSwapchainKHR.originalFn(device, pCreateInfo, pAllocator, pSwapchain);
}

static CHook<VkResult VKAPI_CALL(uintptr_t)> oPresentImage;
static VkResult VKAPI_CALL hkPresentImage(uintptr_t ecx) {
    if (ecx && !oAcquireNextImageKHR.originalFn) {
        uintptr_t m_device = *reinterpret_cast<uintptr_t*>(ecx + 0x10);
        if (m_device) {
            if (!CreateDeviceVK( )) {
                LOG("[katehook] CreateDeviceVK() failed\n");
                return oPresentImage.originalFn(ecx);
            }

            void* fnAcquireNextImageKHR = *reinterpret_cast<void**>(m_device + 0x458);
            void* fnQueuePresentKHR = *reinterpret_cast<void**>(m_device + 0x460);
            void* fnCreateSwapchainKHR = *reinterpret_cast<void**>(m_device + 0x440);

            if (g_FakeDevice) {
                vkDestroyDevice(g_FakeDevice, g_Allocator);
                g_FakeDevice = NULL;
            }

            if (fnAcquireNextImageKHR) {
                // Hook
				LOG("[+] Vulkan: fnAcquireNextImageKHR: %p\n", fnAcquireNextImageKHR);
				LOG("[+] Vulkan: fnQueuePresentKHR: %p\n", fnQueuePresentKHR);
				LOG("[+] Vulkan: fnCreateSwapchainKHR: %p\n", fnCreateSwapchainKHR);

				HOOK(AcquireNextImageKHR);
                HOOK(QueuePresentKHR);
                HOOK(CreateSwapchainKHR);
            }
        }
    }

    return oPresentImage.originalFn(ecx);
}

namespace Hooks {
    namespace Vulkan {
        void Hook() {
            const auto libDXVK = dlopen("libdxvk_d3d9.so", RTLD_LAZY | RTLD_NOLOAD);
            if (!libDXVK) return;
            void* fnPresentImage = dlsym(libDXVK, "_ZN4dxvk2vk9Presenter12presentImageEv");
            if (fnPresentImage) {
                HOOK(PresentImage);
            }
        }

        void Unhook() {
            if (ImGui::GetCurrentContext()) {
                if (ImGui::GetIO().BackendRendererUserData) ImGui_ImplVulkan_Shutdown();

                ImGui_ImplSDL2_Shutdown();
                ImGui::DestroyContext();
            }

            CleanupDeviceVulkan();
        }
    }  // namespace Vulkan
}

static void CleanupRenderTarget( ) {
	for (uint32_t i = 0; i < 8; ++i) {
		if (g_Frames[i].CommandBuffer) { vkFreeCommandBuffers(g_Device, g_Frames[i].CommandPool, 1, &g_Frames[i].CommandBuffer); g_Frames[i].CommandBuffer = VK_NULL_HANDLE; }
		if (g_Frames[i].CommandPool) { vkDestroyCommandPool(g_Device, g_Frames[i].CommandPool, g_Allocator); g_Frames[i].CommandPool = VK_NULL_HANDLE; }
		if (g_Frames[i].BackbufferView) { vkDestroyImageView(g_Device, g_Frames[i].BackbufferView, g_Allocator); g_Frames[i].BackbufferView = VK_NULL_HANDLE; }
		if (g_Frames[i].Framebuffer) { vkDestroyFramebuffer(g_Device, g_Frames[i].Framebuffer, g_Allocator); g_Frames[i].Framebuffer = VK_NULL_HANDLE; }
	}
}

static void CleanupDeviceVulkan( ) {
	CleanupRenderTarget( );

	if (g_DescriptorPool) { vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator); g_DescriptorPool = NULL; }
	if (g_Instance) { vkDestroyInstance(g_Instance, g_Allocator); g_Instance = NULL; }

	g_ImageExtent = {};
	g_Device = NULL;
}

static void RenderImGui_Vulkan(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    if (!g_Device || !Hooks::SDL::windowptr) return;

    if (!ImGui::GetCurrentContext()) {
		LOG("SDL_Window has not been found! Try alt tabbing...");
		return;
		/*
        ImGui::CreateContext();

        ImGui::StyleColorsDark();

        ImFontConfig fontConfig;
        fontConfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_MonoHinting | ImGuiFreeTypeBuilderFlags_Monochrome;
        Menu::menuFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(tahoma_compressed_data, tahoma_compressed_size,
                                                                              14, &fontConfig);

        Menu::weaponFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(&weaponIcons[0], weaponIcons.size(), 14);
        ImGuiFreeType::BuildFontAtlas(ImGui::GetIO().Fonts, 0x0);

        ImGui::GetStyle().WindowPadding = ImVec2(6, 6);
        ImGui::GetStyle().FramePadding = ImVec2(4, 0);
        ImGui::GetStyle().WindowRounding = 1;
        ImGui::GetStyle().ChildRounding = 1;
        ImGui::GetStyle().FrameRounding = 1;
        ImGui::GetStyle().WindowBorderSize = 0;
        ImGui::GetStyle().FrameBorderSize = 1;
        ImGui::GetStyle().ScrollbarRounding = 0;
        ImGui::GetStyle().ScrollbarSize = 3;
        ImGui::GetStyle().WindowMinSize = ImVec2(4, 4);

        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.11f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);

        ImGui_ImplSDL2_InitForVulkan(Hooks::SDL::windowptr);

        ImGuiIO& io = ImGui::GetIO();

        // io.IniFilename = nullptr;
        // io.LogFilename = nullptr;
		*/
    }

    for (uint32_t i = 0; i < pPresentInfo->swapchainCount; ++i) {
        VkSwapchainKHR swapchain = pPresentInfo->pSwapchains[i];
        if (g_Frames[0].Framebuffer == VK_NULL_HANDLE) {
            CreateRenderTarget(g_Device, swapchain);
        }

        ImGui_ImplVulkanH_Frame* fd = &g_Frames[pPresentInfo->pImageIndices[i]];
        {
            vkResetCommandBuffer(fd->CommandBuffer, 0);

            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(fd->CommandBuffer, &info);
        }
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = g_RenderPass;
            info.framebuffer = fd->Framebuffer;
            if (g_ImageExtent.width == 0 || g_ImageExtent.height == 0) {
                // We don't know the window size the first time. So we just set it to 4K.
                info.renderArea.extent.width = 3840;
                info.renderArea.extent.height = 2160;
            } else {
                info.renderArea.extent = g_ImageExtent;
            }

            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        if (!ImGui::GetIO().BackendRendererUserData) {
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = g_Instance;
            init_info.PhysicalDevice = g_PhysicalDevice;
            init_info.Device = g_Device;
            init_info.QueueFamily = g_QueueFamily;
            init_info.Queue = queue;
            init_info.PipelineCache = g_PipelineCache;
            init_info.DescriptorPool = g_DescriptorPool;
            init_info.Subpass = 0;
            init_info.MinImageCount = g_MinImageCount;
            init_info.ImageCount = g_MinImageCount;
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = g_Allocator;
            ImGui_ImplVulkan_Init(&init_info, g_RenderPass);

            ImGui_ImplVulkan_CreateFontsTexture(fd->CommandBuffer);
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(Hooks::SDL::windowptr);
        ImGui::NewFrame();

        // ImGui::ShowDemoWindow( );
        Menu::render(Hooks::SDL::windowptr);

        ImGui::Render();

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);
        {
            VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;

            vkEndCommandBuffer(fd->CommandBuffer);
            vkQueueSubmit(queue, 1, &info, NULL);
        }
    }
}
