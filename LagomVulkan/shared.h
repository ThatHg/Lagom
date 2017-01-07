#pragma once

#include "platform.h"

#include <iostream>
#include <assert.h>

void error_check(VkResult result);

uint32_t find_memory_type_index(const VkPhysicalDeviceMemoryProperties* gpu_memory_properties, const VkMemoryRequirements* memory_requirements, const VkMemoryPropertyFlags required_properties);