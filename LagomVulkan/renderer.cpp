#include "BUILD_OPTIONS.h"

#include "renderer.h"
#include "shared.h"
#include "window.h"

#include <vector>
#include <iostream>
#include <assert.h>
#include <sstream>

Renderer::Renderer()
{
    _setup_layers_and_extensions();
    _setup_debug();
    _init_instance();
    _init_debug();
    _init_device();
}

Renderer::~Renderer()
{
    delete _window;

    _deinit_device();
    _deinit_debug();
    _deinit_instance();
}

Window * Renderer::create_window(uint32_t size_x, uint32_t size_y, std::string name)
{
    _window = new Window(this, size_x, size_y, name);
    return _window;
}

bool Renderer::run()
{
    if (nullptr != _window) {
        return _window->update();
    }
    return true;
}

const VkPhysicalDevice Renderer::get_vulkan_physical_device() const
{
    return _gpu;
}

const VkInstance Renderer::get_vulkan_instance() const
{
    return _instance;
}

const VkDevice Renderer::get_vulkan_device() const
{
    return _device;
}

const VkQueue Renderer::get_vulkan_queue() const
{
    return _queue;
}

const uint32_t Renderer::get_vulkan_graphics_family_index() const
{
    return _graphics_family_index;
}

const VkPhysicalDeviceProperties & Renderer::get_vulkan_physical_device_properties() const
{
    return _gpu_properties;
}

const VkPhysicalDeviceMemoryProperties & Renderer::get_vulkan_physical_device_memory_properties() const
{
    return _gpu_memory_properties;
}

void Renderer::_setup_layers_and_extensions()
{
    //_instance_extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
    _instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    _instance_extensions.push_back(PLATFORM_SURFACE_EXTENSION_NAME);

    _device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void Renderer::_init_instance()
{
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.apiVersion = VK_MAKE_VERSION(1,0,3);
    application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.pApplicationName = "Lagomt Vulkan";

    VkInstanceCreateInfo instance_create_info {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.enabledLayerCount = (uint32_t)_instance_layers.size();
    instance_create_info.ppEnabledLayerNames = _instance_layers.data();
    instance_create_info.enabledExtensionCount = (uint32_t)_instance_extensions.size();
    instance_create_info.ppEnabledExtensionNames = _instance_extensions.data();
    instance_create_info.pNext = &_debug_callback_create_info;

    error_check(vkCreateInstance(&instance_create_info, nullptr, &_instance));
    
}

void Renderer::_deinit_instance()
{
    vkDestroyInstance(_instance, nullptr);
    _instance = nullptr;
}

void Renderer::_init_device()
{
    // Find the right GPU. We take the first GPU in our physical device list, for now.
    {
        uint32_t gpu_count = 0;
        vkEnumeratePhysicalDevices(_instance, &gpu_count, nullptr);
        std::vector<VkPhysicalDevice> gpu_list( gpu_count );
        vkEnumeratePhysicalDevices(_instance, &gpu_count, gpu_list.data());
        _gpu = gpu_list[0];
        vkGetPhysicalDeviceProperties(_gpu, &_gpu_properties);
        vkGetPhysicalDeviceMemoryProperties(_gpu, &_gpu_memory_properties);
    }
    // Find family index with graphics bit support
    {
        uint32_t family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &family_count, nullptr);
        std::vector<VkQueueFamilyProperties> family_property_list(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(_gpu, &family_count, family_property_list.data());
        bool has_graphics_bit = false;
        for (size_t i = 0; i < family_count; ++i) {
            if (family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                has_graphics_bit = true;
                _graphics_family_index = (uint32_t)i;
            }
        }

        if (!has_graphics_bit) {
            assert(0 && "[Vulkan:Error] Queue family supporting graphics bit not found.");
            std::exit(-1);
        }
    }
    // List available instance layers installed in the system
    {
        uint32_t layer_count = 0;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> layer_property_list(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, layer_property_list.data());
        std::cout << "Instance Layers: \n";
        for (int i = 0; i < layer_property_list.size(); ++i) {
            std::cout << "  " << layer_property_list[i].layerName << "\t\t" << layer_property_list[i].description << std::endl;
        }
        std::cout << std::endl;
    }
    // List available device layers installed in the system
    {
        uint32_t layer_count = 0;
        vkEnumerateDeviceLayerProperties(_gpu, &layer_count, nullptr);
        std::vector<VkLayerProperties> layer_property_list(layer_count);
        vkEnumerateDeviceLayerProperties(_gpu, &layer_count, layer_property_list.data());
        std::cout << "Device Layers: \n";
        for (int i = 0; i < layer_property_list.size(); ++i) {
            std::cout << "  " << layer_property_list[i].layerName << "\t\t" << layer_property_list[i].description << std::endl;
        }
        std::cout << std::endl;
    }

    float queue_priorities[] = { 1.0f };
    VkDeviceQueueCreateInfo device_queue_create_info {};
    device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_create_info.queueFamilyIndex = _graphics_family_index;
    device_queue_create_info.queueCount = 1;
    device_queue_create_info.pQueuePriorities = queue_priorities;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &device_queue_create_info;
    device_create_info.enabledLayerCount = (uint32_t)_device_layers.size();
    device_create_info.ppEnabledLayerNames = _device_layers.data();
    device_create_info.enabledExtensionCount = (uint32_t)_device_extensions.size();
    device_create_info.ppEnabledExtensionNames = _device_extensions.data();

    error_check(vkCreateDevice(_gpu, &device_create_info, nullptr, &_device));

    vkGetDeviceQueue(_device, _graphics_family_index, 0, &_queue);

}

void Renderer::_deinit_device()
{
    vkDestroyDevice(_device, nullptr);
}

#if BUILD_ENABLE_VULKAN_DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT obj_type,
    uint64_t src_obj,
    size_t location,
    int32_t msg_code,
    const char* layer_prefix,
    const char* msg,
    void* user_data
    ) 
{
    std::ostringstream stream;

    stream << "[Vulkan Debug] ";
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        stream << "[Info] ";
    }
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        stream << "[Warning] ";
    }
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        stream << "[Performance] ";
    }
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        stream << "[Error] ";
    }
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        stream << "[Debug] ";
    }
    stream << "@[" << layer_prefix << "]:";
    stream << msg << std::endl;
    std::cout << stream.str();

#if defined(_WIN32)
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        MessageBox(NULL, (LPCWSTR)stream.str().c_str(), (LPCWSTR)"Vulkan Error!", 0);
    }
#endif 

    return false;
}
void Renderer::_setup_debug()
{
    _debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    _debug_callback_create_info.pfnCallback = VulkanDebugCallback;
    _debug_callback_create_info.flags =
        //VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
        VK_DEBUG_REPORT_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT |
        //VK_DEBUG_REPORT_DEBUG_BIT_EXT |
        0;

    _instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
    _instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    _device_layers.push_back("VK_LAYER_LUNARG_standard_validation");
}

// Debug function pointers.
PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = nullptr;
void Renderer::_init_debug()
{
    fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
    fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
    if (nullptr == fvkCreateDebugReportCallbackEXT || nullptr == fvkDestroyDebugReportCallbackEXT) {
        assert(0 && "[Vulkan:Error] Can't fetch debug function pointers.");
        std::exit(-1);
    }

    fvkCreateDebugReportCallbackEXT(_instance, &_debug_callback_create_info, nullptr, &_debug_report);
}

void Renderer::_deinit_debug()
{
    fvkDestroyDebugReportCallbackEXT(_instance, _debug_report, nullptr);
    _debug_report = VK_NULL_HANDLE;
}
#else

void Renderer::_setup_debug() {};
void Renderer::_init_debug() {};
void Renderer::_deinit_debug() {};

#endif