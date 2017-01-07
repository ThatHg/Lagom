#include "window.h"
#include "renderer.h"
#include "shared.h"
#include <array>

Window::Window(Renderer* renderer, uint32_t size_x, uint32_t size_y, std::string name)
{
    _renderer = renderer;
    _surface_size_x = size_x;
    _surface_size_y = size_y;
    _window_name = name;

    _init_os_window();
    _init_surface();
    _init_swapchain();
    _init_swapchain_images();
    _init_depth_stencil_image();
    _init_render_pass();
}

Window::~Window()
{
    _deinit_render_pass();
    _deinit_depth_stencil_image();
    _deinit_swapchain_images();
    _deinit_swapchain();
    _deinit_surface();
    _deinit_os_window();
}

void Window::close()
{
    _window_should_run = false;
}

bool Window::update()
{
    _update_os_window();
    return _window_should_run;
}

void Window::_init_surface() {
    _init_os_surface();
    VkPhysicalDevice gpu = _renderer->get_vulkan_physical_device();
    VkBool32 WSI_supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(gpu, _renderer->get_vulkan_graphics_family_index(), _surface, &WSI_supported);
    if (!WSI_supported) {
        assert(0 && "WSI not supported");
        std::exit(-1);
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, _surface, &_surface_capabilities);

    if (_surface_capabilities.currentExtent.width < UINT32_MAX) {
        _surface_size_x = _surface_capabilities.currentExtent.width;
        _surface_size_y = _surface_capabilities.currentExtent.height;
    }

    {
        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, _surface, &format_count, nullptr);
        if (format_count == 0) {
            assert(0 && "Surface format missing.");
            std::exit(-1);
        }
        std::vector<VkSurfaceFormatKHR> formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, _surface, &format_count, formats.data());
        if (formats[0].format == VK_FORMAT_UNDEFINED) {
            _surface_format.format = VK_FORMAT_B8G8R8A8_UNORM;
            _surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }
        else {
            _surface_format = formats[0];
        }
    }
}

void Window::_deinit_surface() {
    vkDestroySurfaceKHR(_renderer->get_vulkan_instance(), _surface, nullptr);
}

void Window::_init_swapchain()
{
    if (_swapchain_image_count < _surface_capabilities.minImageCount + 1) {
        _swapchain_image_count = _surface_capabilities.minImageCount + 1;
    }
    if (_surface_capabilities.maxImageCount > 0) {
        if (_swapchain_image_count > _surface_capabilities.maxImageCount) {
            _swapchain_image_count = _surface_capabilities.maxImageCount;
        }
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    {
        uint32_t present_mode_count = 0;
        error_check(vkGetPhysicalDeviceSurfacePresentModesKHR(_renderer->get_vulkan_physical_device(), _surface, &present_mode_count, nullptr));
        std::vector<VkPresentModeKHR> present_mode_list(present_mode_count);
        error_check(vkGetPhysicalDeviceSurfacePresentModesKHR(_renderer->get_vulkan_physical_device(), _surface, &present_mode_count, present_mode_list.data()));

        // Find the most usefull present mode for gaming.
        // Mailbox is what we want.
        for (uint32_t i = 0; i < present_mode_count; ++i) {
            if (present_mode_list[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
                present_mode = present_mode_list[i];
            }
        }
    }

    VkSwapchainCreateInfoKHR swapchain_create_info{};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = _surface;
    swapchain_create_info.minImageCount = _swapchain_image_count;
    swapchain_create_info.imageFormat = _surface_format.format;
    swapchain_create_info.imageColorSpace = _surface_format.colorSpace;
    swapchain_create_info.imageExtent.width = _surface_size_x;
    swapchain_create_info.imageExtent.height = _surface_size_y;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = nullptr;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    error_check(vkCreateSwapchainKHR(_renderer->get_vulkan_device(), &swapchain_create_info, nullptr, &_swapchain));

    error_check(vkGetSwapchainImagesKHR(_renderer->get_vulkan_device(), _swapchain, &_swapchain_image_count, nullptr));
}

void Window::_deinit_swapchain()
{
    vkDestroySwapchainKHR(_renderer->get_vulkan_device(), _swapchain, nullptr);
}

void Window::_init_swapchain_images()
{
    _swapchain_images.resize(_swapchain_image_count);
    _swapchain_image_views.resize(_swapchain_image_count);

    error_check(vkGetSwapchainImagesKHR(_renderer->get_vulkan_device(), _swapchain, &_swapchain_image_count, _swapchain_images.data()));

    for (uint32_t i = 0; i < _swapchain_image_count; ++i) {
        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = _swapchain_images[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = _surface_format.format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        error_check(vkCreateImageView(_renderer->get_vulkan_device(), &image_view_create_info, nullptr, &_swapchain_image_views[i]));
    }
}

void Window::_deinit_swapchain_images()
{
    for (uint32_t i = 0; i < _swapchain_image_count; ++i) {
        vkDestroyImageView(_renderer->get_vulkan_device(), _swapchain_image_views[i], nullptr);
    }
}

void Window::_init_depth_stencil_image()
{
    {
        std::vector<VkFormat> try_formats{ 
            VK_FORMAT_D32_SFLOAT_S8_UINT, 
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D16_UNORM
        };

        for (int i = 0; i < try_formats.size(); ++i) {
            VkFormatProperties format_properties{};
            vkGetPhysicalDeviceFormatProperties(_renderer->get_vulkan_physical_device(), try_formats[i], &format_properties);
            if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                _depth_stencil_format = try_formats[i];
            }
        }
        if (_depth_stencil_format == VK_FORMAT_UNDEFINED) {
            assert(0 && "Depth stencil format Undefined");
            exit(-1);
        }
        if ((_depth_stencil_format == VK_FORMAT_D32_SFLOAT_S8_UINT) || 
            (_depth_stencil_format == VK_FORMAT_D24_UNORM_S8_UINT) ||
            (_depth_stencil_format == VK_FORMAT_D16_UNORM_S8_UINT) ||
            (_depth_stencil_format == VK_FORMAT_S8_UINT) ) {
            _stencil_available = true;
        }
    }

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = _depth_stencil_format;
    image_create_info.extent.width = _surface_size_x;
    image_create_info.extent.height = _surface_size_y;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    error_check(vkCreateImage(_renderer->get_vulkan_device(), &image_create_info, nullptr, &_depth_stencil_image));

    VkMemoryRequirements image_memory_requirements{};
    vkGetImageMemoryRequirements(_renderer->get_vulkan_device(), _depth_stencil_image, &image_memory_requirements);
    uint32_t memory_index = find_memory_type_index(&_renderer->get_vulkan_physical_device_memory_properties(), &image_memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = image_memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = memory_index;

    error_check(vkAllocateMemory(_renderer->get_vulkan_device(), &memory_allocate_info, nullptr, &_depth_stencil_image_memory));
    error_check(vkBindImageMemory(_renderer->get_vulkan_device(), _depth_stencil_image, _depth_stencil_image_memory, 0));

    VkImageViewCreateInfo image_view_create_info{};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = _depth_stencil_image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = _depth_stencil_format;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (_stencil_available ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    error_check(vkCreateImageView(_renderer->get_vulkan_device(), &image_view_create_info, nullptr, &_depth_stencil_image_view));
}

void Window::_deinit_depth_stencil_image()
{
    vkDestroyImageView(_renderer->get_vulkan_device(), _depth_stencil_image_view, nullptr);
    vkFreeMemory(_renderer->get_vulkan_device(), _depth_stencil_image_memory, nullptr);
    vkDestroyImage(_renderer->get_vulkan_device(), _depth_stencil_image, nullptr);
}

void Window::_init_render_pass()
{
    std::array<VkAttachmentDescription, 2> attachments{};
    attachments[0].flags = 0;
    attachments[0].format = _depth_stencil_format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments[1].flags = 0;
    attachments[1].format = _surface_format.format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference sub_pass_0_stencil_attachments{};
    sub_pass_0_stencil_attachments.attachment = 0;
    sub_pass_0_stencil_attachments.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    std::array<VkAttachmentReference, 1> sub_pass_0_color_attachments{};
    sub_pass_0_color_attachments[0].attachment = 1;
    sub_pass_0_color_attachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::array<VkSubpassDescription, 1> sub_passes{};
    sub_passes[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub_passes[0].colorAttachmentCount = sub_pass_0_color_attachments.size();
    sub_passes[0].pColorAttachments = sub_pass_0_color_attachments.data();
    sub_passes[0].pDepthStencilAttachment = &sub_pass_0_stencil_attachments;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = attachments.size();
    render_pass_create_info.pAttachments = attachments.data();
    render_pass_create_info.subpassCount = sub_passes.size();
    render_pass_create_info.pSubpasses = sub_passes.data();

    error_check(vkCreateRenderPass(_renderer->get_vulkan_device(), &render_pass_create_info, nullptr, &_render_pass));
}

void Window::_deinit_render_pass()
{
    vkDestroyRenderPass(_renderer->get_vulkan_device(), _render_pass, nullptr);
}
