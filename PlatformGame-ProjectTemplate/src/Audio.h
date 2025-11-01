#pragma once

#include "Module.h"
#include <SDL3/SDL.h>
#include <vector>
#include <string>

#define DEFAULT_MUSIC_FADE_TIME 2.0f

struct _Mix_Music;

class Audio : public Module
{
public:

    Audio();

    // Destructor
    virtual ~Audio();

    // Called before render is available
    bool Awake();

    // Called before quitting
    bool CleanUp();

    // Play a music file
    bool PlayMusic(const char* path, float fadeTime = DEFAULT_MUSIC_FADE_TIME);

    // Load a WAV in memory
    int LoadFx(const char* path);

    // Play a previously loaded WAV
    bool PlayFx(int fx, int repeat = 0);

    // Play a previously loaded WAV with custom volume (0.0 to 1.0)
    bool PlayFx(int fx, float volume, int repeat = 0);

    // Set volume for music (0.0 to 1.0)
    void SetMusicVolume(float volume);

    // Set volume for sound effects (0.0 to 1.0)
    void SetFxVolume(float volume);

    // Stop all sounds on SFX stream
    void StopAllFx();

private:

    struct SoundData {
        SDL_AudioSpec spec{};  // source format
        Uint8* buf{ nullptr };
        Uint32 len{ 0 };  // bytes
    };

    // Device and default output format
    SDL_AudioDeviceID device_{ 0 };
    SDL_AudioSpec     device_spec_{};

    // Streams
    SDL_AudioStream* music_stream_{ nullptr }; // for background music (single)
    SDL_AudioStream* sfx_stream_{ nullptr };   // simple shared SFX stream

    // Loaded sounds
    SoundData music_data_{};
    std::vector<SoundData> sfx_; // 1-based indexing outwardly

    // Volume controls (0.0 to 1.0)
    float music_volume_{ 1.0f };
    float fx_volume_{ 1.0f };

    // helpers
    bool LoadWavFile(const char* path, SoundData& out);
    void FreeSound(SoundData& s);
    bool EnsureDeviceOpen();
    bool EnsureStreams();
};