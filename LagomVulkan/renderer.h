#pragma once
#include "platform.h"
#include <vector>

class Window;

class Renderer {
public:
    Renderer();
    ~Renderer();

    Window* create_window(uint32_t size_x, uint32_t size_y, std::string name);

    bool run();
private:
    void _init_instance();
    void _deinit_instance();
    void _init_device();
    void _deinit_device();
    void _setup_debug();
    void _init_debug();
    void _deinit_debug();

    VkInstance _instance = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkPhysicalDevice _gpu = VK_NULL_HANDLE;
    VkQueue _queue = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties _gpu_properties = {};

    uint32_t _graphics_family_index = 0;

    Window* _window = nullptr;
    
    std::vector<const char*> _instance_layers;
    std::vector<const char*> _instance_extensions;
    std::vector<const char*> _device_layers;
    std::vector<const char*> _device_extensions;

    VkDebugReportCallbackEXT _debug_report = VK_NULL_HANDLE;
    VkDebugReportCallbackCreateInfoEXT _debug_callback_create_info{};
};