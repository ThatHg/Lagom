#pragma once
class Audio {
public:
    virtual ~Audio() {}
    virtual void play_sound(int soundId) = 0;
    virtual void stop_sound(int soundId) = 0;
};

class NullAudio : public Audio
{
public:
    virtual void play_sound(int soundID) { /* Do nothing. */ }
    virtual void stop_sound(int soundID) { /* Do nothing. */ }
};
