#pragma once

#include "platform.h"

#include <string>
#include <vector>

class Renderer;

class Window {
public:
    Window(Renderer * renderer, uint32_t size_x, uint32_t size_y, std::string name);
    ~Window();

    void close();
    bool update();

private:
    void _init_os_window();
    void _deinit_os_window();
    void _update_os_window();
    void _init_os_surface();

    void _init_surface();
    void _deinit_surface();

    void _init_swapchain();
    void _deinit_swapchain();

    void _init_swapchain_images();
    void _deinit_swapchain_images();

    void _init_depth_stencil_image();
    void _deinit_depth_stencil_image();

    void _init_render_pass();
    void _deinit_render_pass();

    Renderer* _renderer = nullptr;

    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    VkRenderPass _render_pass = VK_NULL_HANDLE;

    uint32_t _surface_size_x = 512;
    uint32_t _surface_size_y = 512;
    std::string _window_name = "Lagomt Vulkan";
    uint32_t _swapchain_image_count = 2;

    VkSurfaceFormatKHR _surface_format = {};
    VkSurfaceCapabilitiesKHR _surface_capabilities = {};
    VkFormat _depth_stencil_format = VK_FORMAT_UNDEFINED;

    std::vector<VkImage> _swapchain_images;
    std::vector<VkImageView> _swapchain_image_views;
    VkImage _depth_stencil_image = VK_NULL_HANDLE;
    VkDeviceMemory _depth_stencil_image_memory = VK_NULL_HANDLE;
    VkImageView _depth_stencil_image_view = VK_NULL_HANDLE;

    bool _stencil_available = false;

    bool _window_should_run = true;

#if VK_USE_PLATFORM_WIN32_KHR
    HINSTANCE _win32_instance = NULL;
    HWND _win32_window = NULL;
    std::string _win32_class_name;
    static uint64_t _win32_class_id_counter;
#endif
};