#include "window.h"

Window::Window(uint32_t size_x, uint32_t size_y, std::string name)
{
    _surface_size_x = size_x;
    _surface_size_y = size_y;
    _window_name = name;

    _init_os_window();
}

Window::~Window()
{
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
