#pragma once
#include "audio.h"
class Locator {
public:
    static void initialize() { Locator::_audio = &Locator::_null_audio; }

    static Audio& get_audio();
    static void register_audio(Audio* service);

private:
    static Audio* _audio;
    static NullAudio _null_audio ;
};