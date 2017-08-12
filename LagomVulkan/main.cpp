#include "renderer.h"
#include "window.h"
#include "shared.h"
#include "locator.h"
#include "audio_open_al.h"
#include <array>
#include <chrono>

constexpr double PI = 3.14159265358979323846;
constexpr double CIRCLE_RAD = PI * 2;
constexpr double CIRCLE_THIRD = CIRCLE_RAD / 3.0;
constexpr double CIRCLE_THIRD_1 = 0;
constexpr double CIRCLE_THIRD_2 = CIRCLE_THIRD;
constexpr double CIRCLE_THIRD_3 = CIRCLE_THIRD * 2;

int main() {
    /*AudioOpenAL* a = new AudioOpenAL();
    Locator::initialize();
    Locator::register_audio(a);
    delete a;*/

    Renderer r;
    Window* w = r.create_window(1280, 720, "Lagomt Vulkan");

    VkCommandPool command_pool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_create_info.queueFamilyIndex = r.get_vulkan_graphics_family_index();
    error_check(vkCreateCommandPool(r.get_vulkan_device(), &pool_create_info, nullptr, &command_pool));

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;
    error_check( vkAllocateCommandBuffers(r.get_vulkan_device(), &command_buffer_allocate_info, &command_buffer));

    VkSemaphore render_complete_semaphore = VK_NULL_HANDLE;
    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(r.get_vulkan_device(), &semaphore_create_info, nullptr, &render_complete_semaphore);

    float color_rotation = 0.0f;
    auto timer = std::chrono::steady_clock();
    auto last_time = timer.now();
    uint64_t frame_counter = 0;
    uint64_t fps = 0;

    while (r.run()) {
        // Cpu logic
        ++frame_counter;
        if (last_time + std::chrono::seconds(1) < timer.now()) {
            last_time = timer.now();
            fps = frame_counter;
            frame_counter = 0; 
            std::cout << "FPS: " << fps << std::endl;
        }

        // Begin render
        w->begin_render();
        // Record command buffer
        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        error_check(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

        VkRect2D render_area{};
        render_area.offset.x = 0;
        render_area.offset.y = 0;
        render_area.extent = w->get_vulkan_surface_size();

        color_rotation += 0.01f;

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].depthStencil.depth = 0.0f;
        clear_values[0].depthStencil.stencil = 0;
        clear_values[1].color.float32[0] = std::sin(color_rotation + (float)CIRCLE_THIRD_1) * 0.5f + 0.5f;
        clear_values[1].color.float32[1] = std::sin(color_rotation + (float)CIRCLE_THIRD_2) * 0.5f + 0.5f;
        clear_values[1].color.float32[2] = std::sin(color_rotation + (float)CIRCLE_THIRD_3) * 0.5f + 0.5f;
        clear_values[1].color.float32[3] = 1.0f;

        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = w->get_vulkan_render_pass();
        render_pass_begin_info.framebuffer = w->get_vulkan_active_framebuffer();
        render_pass_begin_info.renderArea = render_area;
        render_pass_begin_info.clearValueCount = (uint32_t)clear_values.size();
        render_pass_begin_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdEndRenderPass(command_buffer);

        error_check(vkEndCommandBuffer(command_buffer));
        // Submit command buffer
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitSemaphores = nullptr;
        submit_info.pWaitDstStageMask = nullptr;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_complete_semaphore;

        error_check(vkQueueSubmit(r.get_vulkan_queue(), 1, &submit_info, VK_NULL_HANDLE));

        // End render
        w->end_render({ render_complete_semaphore });
    }
    error_check(vkQueueWaitIdle(r.get_vulkan_queue()));
    vkDestroySemaphore(r.get_vulkan_device(), render_complete_semaphore, nullptr);
    vkDestroyCommandPool(r.get_vulkan_device(), command_pool, nullptr);

    return 0;
}