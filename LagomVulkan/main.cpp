#define GLFW_INCLUDE_VULKAN
#include "renderer.h"
int main() {
    Renderer r;
    r.create_window(1280, 720, "Lagomt Vulkan");

    while (r.run()) {

    }

    return 0;
}