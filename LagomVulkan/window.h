#pragma once

#include <string>

class Window {
public:
    Window(uint32_t size_x, uint32_t size_y, std::string name);
    ~Window();

    void close();
    bool update();

private:
    void _init_os_window();
    void _deinit_os_window();
    void _update_os_window();
    void _init_os_surface();

    uint32_t _surface_size_x = 512;
    uint32_t _surface_size_y = 512;
    std::string _window_name = "Lagomt Vulkan";

    bool _window_should_run = true;

#if VK_USE_PLATFORM_WIN32_KHR
    HINSTANCE _win32_instance = NULL;
    HWND _win32_window = NULL;
    std::string _win32_class_name;
    static uint64_t _win32_class_id_counter;
#endif
};