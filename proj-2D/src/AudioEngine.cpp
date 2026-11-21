// ============================================================================
// File: src/AudioEngine.cpp
// Progetto: Astro-Miner 2D: Cosmic Coin Rush
// Disciplina: Computer Graphics - Università di Bologna (UniBo)
// Descrizione: Implementazione del sintetizzatore audio procedurale in tempo reale.
// ============================================================================

#include "AudioEngine.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <iostream>

#ifdef __APPLE__
#include <AudioToolbox/AudioToolbox.h>

static const int SAMPLE_RATE = 44100;
static const int BUFFER_SIZE = 2048;

static void audioQueueCallback(void* userData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
{
    AudioEngine* engine = static_cast<AudioEngine*>(userData);
    int numFrames = inBuffer->mAudioDataBytesCapacity / sizeof(short);

    std::vector<float> floatBuf(numFrames, 0.0f);
    engine->renderAudio(floatBuf.data(), numFrames, 1);

    short* outPcm = static_cast<short*>(inBuffer->mAudioData);
    for (int i = 0; i < numFrames; ++i)
    {
        float s = std::clamp(floatBuf[i], -1.0f, 1.0f);
        outPcm[i] = static_cast<short>(s * 32760.0f);
    }

    inBuffer->mAudioDataByteSize = inBuffer->mAudioDataBytesCapacity;
    AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, nullptr);
}
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, playWebAudioTone, (int effectType, float volume), {
    try {
        var AudioContext = window.AudioContext || window.webkitAudioContext;
        if (!window.gameAudioCtx) {
            window.gameAudioCtx = new AudioContext();
        }
        var ctx = window.gameAudioCtx;
        if (ctx.state === 'suspended') {
            ctx.resume();
        }

        var osc = ctx.createOscillator();
        var gain = ctx.createGain();
        osc.connect(gain);
        gain.connect(ctx.destination);

        var now = ctx.currentTime;
        gain.gain.setValueAtTime(volume * 0.35, now);

        if (effectType === 0) { // LASER
            osc.type = 'sawtooth';
            osc.frequency.setValueAtTime(840, now);
            osc.frequency.exponentialRampToValueAtTime(160, now + 0.12);
            gain.gain.linearRampToValueAtTime(0.01, now + 0.12);
            osc.start(now);
            osc.stop(now + 0.13);
        } else if (effectType === 1) { // EXPLOSION
            osc.type = 'triangle';
            osc.frequency.setValueAtTime(140, now);
            osc.frequency.exponentialRampToValueAtTime(30, now + 0.3);
            gain.gain.linearRampToValueAtTime(0.01, now + 0.3);
            osc.start(now);
            osc.stop(now + 0.31);
        } else if (effectType === 2) { // COIN
            osc.type = 'sine';
            osc.frequency.setValueAtTime(523.25, now);
            osc.frequency.setValueAtTime(659.25, now + 0.08);
            gain.gain.linearRampToValueAtTime(0.01, now + 0.22);
            osc.start(now);
            osc.stop(now + 0.23);
        } else if (effectType === 3) { // POWERUP
            osc.type = 'triangle';
            osc.frequency.setValueAtTime(440, now);
            osc.frequency.setValueAtTime(554.37, now + 0.08);
            osc.frequency.setValueAtTime(659.25, now + 0.16);
            gain.gain.linearRampToValueAtTime(0.01, now + 0.32);
            osc.start(now);
            osc.stop(now + 0.33);
        } else if (effectType === 4) { // SHIELD DEFLECT
            osc.type = 'square';
            osc.frequency.setValueAtTime(340, now);
            gain.gain.exponentialRampToValueAtTime(0.01, now + 0.16);
            osc.start(now);
            osc.stop(now + 0.17);
        } else if (effectType === 5) { // DRONE HIT
            osc.type = 'sawtooth';
            osc.frequency.setValueAtTime(260, now);
            osc.frequency.linearRampToValueAtTime(90, now + 0.22);
            gain.gain.linearRampToValueAtTime(0.01, now + 0.22);
            osc.start(now);
            osc.stop(now + 0.23);
        } else if (effectType === 6) { // WARP
            osc.type = 'sine';
            osc.frequency.setValueAtTime(220, now);
            osc.frequency.exponentialRampToValueAtTime(1100, now + 0.7);
            gain.gain.linearRampToValueAtTime(0.01, now + 0.75);
            osc.start(now);
            osc.stop(now + 0.76);
        }
    } catch(e) {
        console.warn("WebAudio not allowed yet:", e);
    }
});
#endif

AudioEngine& AudioEngine::instance()
{
    static AudioEngine s_instance;
    return s_instance;
}

AudioEngine::AudioEngine()
{
}

AudioEngine::~AudioEngine()
{
    shutdown();
}

bool AudioEngine::init()
{
    if (m_initialized) return true;

#ifdef __APPLE__
    AudioStreamBasicDescription format = {};
    format.mSampleRate       = SAMPLE_RATE;
    format.mFormatID         = kAudioFormatLinearPCM;
    format.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    format.mBitsPerChannel   = 16;
    format.mChannelsPerFrame = 1;
    format.mBytesPerFrame    = sizeof(short);
    format.mFramesPerPacket  = 1;
    format.mBytesPerPacket   = sizeof(short);

    AudioQueueRef queue = nullptr;
    OSStatus status = AudioQueueNewOutput(&format, audioQueueCallback, this, nullptr, nullptr, 0, &queue);
    if (status != noErr)
    {
        std::cerr << "[AudioEngine] Inizializzazione AudioQueue fallita (OSStatus " << status << ").\n";
        return false;
    }

    m_audioQueue = queue;

    for (int i = 0; i < 3; ++i)
    {
        AudioQueueBufferRef buf = nullptr;
        AudioQueueAllocateBuffer(queue, BUFFER_SIZE * sizeof(short), &buf);
        buf->mAudioDataByteSize = BUFFER_SIZE * sizeof(short);
        std::memset(buf->mAudioData, 0, buf->mAudioDataByteSize);
        AudioQueueEnqueueBuffer(queue, buf, 0, nullptr);
        m_buffers[i] = buf;
    }

    AudioQueueStart(queue, nullptr);
#endif

    m_initialized = true;
    return true;
}

void AudioEngine::shutdown()
{
#ifdef __APPLE__
    if (m_audioQueue)
    {
        AudioQueueRef queue = static_cast<AudioQueueRef>(m_audioQueue);
        AudioQueueStop(queue, true);
        AudioQueueDispose(queue, true);
        m_audioQueue = nullptr;
    }
#endif
    m_initialized = false;
}

void AudioEngine::setMasterVolume(float vol)
{
    m_masterVolume = std::clamp(vol, 0.0f, 1.0f);
}

void AudioEngine::play(SoundEffect effect)
{
    if (m_muted) return;

#ifdef __EMSCRIPTEN__
    playWebAudioTone(static_cast<int>(effect), m_masterVolume);
    return;
#endif

    std::lock_guard<std::mutex> lock(m_voiceMutex);

    ActiveVoice v;
    v.type = effect;
    v.time = 0.0f;

    switch (effect)
    {
    case SoundEffect::LASER:          v.duration = 0.13f; break;
    case SoundEffect::EXPLOSION:      v.duration = 0.35f; break;
    case SoundEffect::COIN:           v.duration = 0.22f; break;
    case SoundEffect::POWERUP:        v.duration = 0.32f; break;
    case SoundEffect::SHIELD_DEFLECT: v.duration = 0.18f; break;
    case SoundEffect::DRONE_HIT:      v.duration = 0.24f; break;
    case SoundEffect::WARP_JUMP:      v.duration = 0.75f; break;
    }

    m_voices.push_back(v);
}

void AudioEngine::renderAudio(float* outputBuffer, int numFrames, int numChannels)
{
    (void)numChannels;
    std::fill(outputBuffer, outputBuffer + numFrames, 0.0f);

    if (m_muted || m_masterVolume <= 0.001f) return;

    std::lock_guard<std::mutex> lock(m_voiceMutex);
    const float dt = 1.0f / 44100.0f;

    for (auto it = m_voices.begin(); it != m_voices.end(); )
    {
        float vTime = it->time;
        float vDur = it->duration;
        SoundEffect vType = it->type;

        for (int i = 0; i < numFrames; ++i)
        {
            float t = vTime + i * dt;
            if (t >= vDur) break;

            float progress = t / vDur;
            float sample = 0.0f;

            if (vType == SoundEffect::LASER)
            {
                // Discesa esponenziale di tono (840 Hz -> 180 Hz)
                float freq = 840.0f * std::pow(0.20f, progress);
                sample = std::sin(t * freq * 6.28318f) * (1.0f - progress);
            }
            else if (vType == SoundEffect::EXPLOSION)
            {
                // Rumore bianco filtrato con decadimento esponenziale
                float noise = (static_cast<float>(rand() % 2000) / 1000.0f) - 1.0f;
                sample = noise * std::exp(-progress * 5.2f);
            }
            else if (vType == SoundEffect::COIN)
            {
                // Arpeggio a 2 note (C5 523Hz -> E5 659Hz)
                float freq = (progress < 0.45f) ? 523.25f : 659.25f;
                sample = std::sin(t * freq * 6.28318f) * (1.0f - progress * 0.9f);
            }
            else if (vType == SoundEffect::POWERUP)
            {
                // Triade ascendente (C5 -> E5 -> G5)
                float freq = (progress < 0.33f) ? 523.25f : (progress < 0.66f ? 659.25f : 783.99f);
                sample = (std::sin(t * freq * 6.28318f) + 0.35f * std::sin(t * freq * 12.5663f)) * (1.0f - progress);
            }
            else if (vType == SoundEffect::SHIELD_DEFLECT)
            {
                // Onda quadra risonante metallica
                float sinVal = std::sin(t * 360.0f * 6.28318f);
                sample = (sinVal > 0.0f ? 0.6f : -0.6f) * std::exp(-progress * 6.5f);
            }
            else if (vType == SoundEffect::DRONE_HIT)
            {
                // Ronzio elettromagnetico bi-armonico
                sample = (0.6f * std::sin(t * 160.0f * 6.28318f) + 0.4f * std::sin(t * 480.0f * 6.28318f)) * (1.0f - progress);
            }
            else if (vType == SoundEffect::WARP_JUMP)
            {
                // Rampa ascendente cosmica verso l'infinito
                float freq = 220.0f + 980.0f * progress * progress;
                sample = std::sin(t * freq * 6.28318f) * (1.0f - progress * 0.7f);
            }

            outputBuffer[i] += sample * m_masterVolume * 0.42f;
        }

        it->time += numFrames * dt;
        if (it->time >= it->duration)
        {
            it = m_voices.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
