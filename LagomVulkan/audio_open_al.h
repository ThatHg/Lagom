#pragma once
#include <al.h>
#include <alc.h>
#include <string>
#include "audio.h"

class AudioOpenAL : public Audio {
public:
    AudioOpenAL();
    virtual ~AudioOpenAL();
    virtual void play_sound(int soundID) { /* Do nothing. */ }
    virtual void stop_sound(int soundID) { /* Do nothing. */ }
private:
    void _init_device();
    void _deinit_device();

    void _init_context();
    void _deinit_context();

    void _init_buffers();
    void _deinit_buffers();

    bool _load_wave(
        const std::string filename, 
        ALuint* buffer, 
        ALsizei* size, 
        ALsizei* frequency, 
        ALenum* format, 
        unsigned char* data);

    ALCdevice* _device = nullptr;
    ALCcontext* _context = nullptr;

    ALuint _buffers[1];

    ALboolean _eax_available = false;
};