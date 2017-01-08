#include "locator.h"

Audio* Locator::_audio;
NullAudio Locator::_null_audio;

Audio& Locator::get_audio()
{
    return *_audio;
}

void Locator::register_audio(Audio* service)
{
    if (service == nullptr)
    {
        _audio = &_null_audio;
    }
    else
    {
        _audio = service;
    }
}
