#pragma once

#include <vector>
#include <asp/sync/Mutex.hpp>
#include <Geode/fmod/fmod.hpp>

#ifndef GEODE_IS_IOS

/// Captures interleaved stereo float PCM from the FMOD master bus.
/// Pauses the master group inside the DSP callback so audio advances
/// in lockstep with video frames (single-pass recording, no drift).
class DSPRecorder {
public:
    static DSPRecorder* get();

    /// Install DSP on master bus and begin capturing.
    void start();
    /// Remove DSP from master bus and stop capturing.
    void stop();

    /// Returns captured PCM data and clears the internal buffer.
    std::span<float> getData();
    
    void tryUnpause(float time) const;

    [[nodiscard]] bool isRecording() const { return m_recording; }

private:
    void init();

    FMOD::DSP*          m_dsp         = nullptr;
    FMOD::ChannelGroup* m_masterGroup = nullptr;
    asp::Mutex<std::vector<float>> m_data;
    bool                m_recording   = false;
};

#endif