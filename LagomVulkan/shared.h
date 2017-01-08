#pragma once

#include "platform.h"

#include <iostream>
#include <assert.h>

void error_check(VkResult result);

uint32_t find_memory_type_index(const VkPhysicalDeviceMemoryProperties* gpu_memory_properties, const VkMemoryRequirements* memory_requirements, const VkMemoryPropertyFlags required_properties);

struct RIFFHeader {
    char chunk_id[4];
    long chunk_size;
    char format[4];
};

struct WAVEFormat {
    char sub_chunk_id[4];
    long sub_chunk_size;
    short audio_format;
    short num_channels;
    long sample_rate;
    long byte_rate;
    short block_align;
    short bits_per_sample;
};

struct WAVEData {
    char sub_chunk_id[4];
    long sub_chunk_2_size;
};