// ============================================================================
// File: include/AudioEngine.h
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Motore Audio Procedurale in tempo reale (Zero Dipendenze Esterne).
//              Sintetizza matematicamente effetti sonori 8-bit/chiptune e sci-fi:
//              frequenze discendenti per laser, rumore bianco per esplosioni,
//              arpeggi armonici per monete e power-up, ronzio per deflessione scudo.
//              Su macOS usa AudioToolbox (AudioQueue); su Web usa Web Audio API.
// ============================================================================

#pragma once

#include <vector>
#include <mutex>
#include <string>

enum class SoundEffect
{
    LASER,
    EXPLOSION,
    COIN,
    POWERUP,
    SHIELD_DEFLECT,
    DRONE_HIT,
    WARP_JUMP
};

struct ActiveVoice
{
    SoundEffect type;
    float time = 0.0f;
    float duration = 0.2f;
};

class AudioEngine
{
public:
    static AudioEngine& instance();

    bool init();
    void shutdown();

    void play(SoundEffect effect);
    void playLaser()          { play(SoundEffect::LASER); }
    void playExplosion()      { play(SoundEffect::EXPLOSION); }
    void playCoin()           { play(SoundEffect::COIN); }
    void playPowerUp()        { play(SoundEffect::POWERUP); }
    void playShieldDeflect()  { play(SoundEffect::SHIELD_DEFLECT); }
    void playDroneHit()       { play(SoundEffect::DRONE_HIT); }
    void playWarp()           { play(SoundEffect::WARP_JUMP); }

    void setMasterVolume(float vol);
    float masterVolume() const { return m_masterVolume; }

    void setMuted(bool muted) { m_muted = muted; }
    bool isMuted() const { return m_muted; }
    bool& mutedRef() { return m_muted; }
    float& volumeRef() { return m_masterVolume; }

    // Generazione del campione audio per il buffer hardware
    void renderAudio(float* outputBuffer, int numFrames, int numChannels);

private:
    AudioEngine();
    ~AudioEngine();

    bool m_initialized = false;
    bool m_muted = false;
    float m_masterVolume = 0.65f;

    std::vector<ActiveVoice> m_voices;
    std::mutex m_voiceMutex;

    // Strutture interne hardware per macOS
    void* m_audioQueue = nullptr;
    void* m_buffers[3] = { nullptr, nullptr, nullptr };
};
