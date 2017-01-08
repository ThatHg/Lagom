#include "audio_open_al.h"
#include "shared.h"
#include <assert.h>
#include <cstdlib>
#include <iostream>

AudioOpenAL::AudioOpenAL()
{
    _init_device();
    _init_context();
    _init_buffers();
}

AudioOpenAL::~AudioOpenAL()
{
    _deinit_buffers();
    _deinit_context();
    _deinit_device();
}

void AudioOpenAL::_init_device()
{
    _device = alcOpenDevice(nullptr);
}

void AudioOpenAL::_deinit_device()
{
    alcCloseDevice(_device);
}

void AudioOpenAL::_init_context()
{
    _context = alcCreateContext(_device, nullptr);
    alcMakeContextCurrent(_context);
}

void AudioOpenAL::_deinit_context()
{
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(_context);
}

void AudioOpenAL::_init_buffers()
{
    _eax_available = alIsExtensionPresent("EAX2.0");
    alGetError();
    alGenBuffers(1, _buffers);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cout << "Error: " << error << " ";
        assert(0 && "Unable to alGenBuffers");
        std::exit(-1);
    }
    unsigned char* data = nullptr;
    ALsizei size;
    ALsizei frequency;
    ALenum format;

    _load_wave("THX48@16.wav", _buffers, &size, &frequency, &format, data);
}

void AudioOpenAL::_deinit_buffers()
{
    alDeleteBuffers(1, _buffers);
}

bool AudioOpenAL::_load_wave(
    const std::string filename, 
    ALuint * buffer, 
    ALsizei * size, 
    ALsizei * frequency, 
    ALenum * format, 
    unsigned char* data)
{
    FILE* audio_file = nullptr;
    RIFFHeader riff_header;
    WAVEFormat wave_format;
    WAVEData wave_data;

    // Open sound file.
    fopen_s(&audio_file, filename.c_str(), "rb");
    if (!audio_file) {
        assert(0 && "Unable to open audio file");
        return false;
    }

    // Read riff header
    fread(&riff_header, sizeof(RIFFHeader), 1, audio_file);

    // Check for sure it has RIFF and WAVE tag
    if ((riff_header.chunk_id[0] != 'R' ||
        riff_header.chunk_id[1] != 'I' ||
        riff_header.chunk_id[2] != 'F' ||
        riff_header.chunk_id[3] != 'F') ||
        (riff_header.format[0] != 'W' ||
            riff_header.format[1] != 'A' ||
            riff_header.format[2] != 'V' ||
            riff_header.format[3] != 'E')) {
        fclose(audio_file);
        assert(0 && "Invalid RIFF or WAVE header");
        return false;
    }

    // Read wave format
    fread(&wave_format, sizeof(WAVEFormat), 1, audio_file);

    // Check for fmt tag
    if (wave_format.sub_chunk_id[0] != 'f' ||
        wave_format.sub_chunk_id[1] != 'm' ||
        wave_format.sub_chunk_id[2] != 't' ||
        wave_format.sub_chunk_id[3] != ' ') {
        fclose(audio_file);
        assert(0 && "Invalid WAVE format");
        return false;
    }

    // Check for extra parameters
    if (wave_format.sub_chunk_size > 16) {
        fseek(audio_file, sizeof(short), SEEK_CUR);
    }

    // Read wave data
    fread(&wave_data, sizeof(WAVEData), 1, audio_file);
    if (wave_data.sub_chunk_id[0] != 'd' ||
        wave_data.sub_chunk_id[1] != 'a' ||
        wave_data.sub_chunk_id[2] != 't' ||
        wave_data.sub_chunk_id[3] != 'a') {
        fclose(audio_file);
        assert(0 && "Invalid WAVE data header");
        return false;
    }

    // Allocate heap memory for data
    data = new unsigned char[wave_data.sub_chunk_2_size];

    // Read audio data 
    if (!fread(data, wave_data.sub_chunk_2_size, 1, audio_file)) {
        fclose(audio_file);
        assert(0 && "Error loading wave data into array");
        return false;
    }

    if (size != nullptr) {
        *size = wave_data.sub_chunk_2_size;
    }
    if (frequency != nullptr) {
        *frequency = wave_format.sample_rate;
    }

    // Figure out audio format
    if (format != nullptr) {
        if (wave_format.num_channels == 1) {
            if (wave_format.bits_per_sample == 8)
                *format = AL_FORMAT_MONO8;
            else if (wave_format.bits_per_sample == 16)
                *format = AL_FORMAT_MONO16;
        }
        else if (wave_format.num_channels == 2) {
            if (wave_format.bits_per_sample == 8)
                *format = AL_FORMAT_STEREO8;
            else if (wave_format.bits_per_sample == 16)
                *format = AL_FORMAT_STEREO16;
        }
    }

    fclose(audio_file);

    alBufferData(_buffers[0], *format, data, *size, *frequency);
    return true;
}
