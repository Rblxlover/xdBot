#include "DSPRecorder.hpp"

#ifndef GEODE_IS_IOS

#include <Geode/binding/FMODAudioEngine.hpp>
#include <Geode/loader/Log.hpp>

using namespace geode::prelude;

DSPRecorder* DSPRecorder::get() {
    static DSPRecorder instance;
    return &instance;
}

void DSPRecorder::init() {
    auto* fmod   = FMODAudioEngine::sharedEngine();
    auto* system = fmod->m_system;
    
    FMOD_DSP_DESCRIPTION desc = {};
    std::strncpy(desc.name, "xdbot_capture", sizeof(desc.name));
    desc.version          = 0x00010000;
    desc.numinputbuffers  = 1;
    desc.numoutputbuffers = 1;
    desc.read = [](FMOD_DSP_STATE*, float* inbuffer, float* outbuffer,
    unsigned int length, int, int* outchannels) -> FMOD_RESULT {
        auto* recorder = DSPRecorder::get();
        if (!recorder->m_recording) return FMOD_OK;
        int channels = *outchannels;
        if (channels <= 0) channels = 2;
        {
            auto guard = recorder->m_data.lock();
            guard->insert(guard->end(), inbuffer, inbuffer + length * channels);
        }
        std::memcpy(outbuffer, inbuffer, length * channels * sizeof(float));
        
        FMOD::ChannelGroup* master = nullptr;
        FMODAudioEngine::sharedEngine()->m_system->getMasterChannelGroup(&master);
        if (master) master->setPaused(true);
        
        return FMOD_OK;
    };
        
        if (system->createDSP(&desc, &m_dsp) != FMOD_OK) {
            log::error("DSPRecorder: failed to create DSP");
            m_dsp = nullptr;
            return;
        }
        system->getMasterChannelGroup(&m_masterGroup);
    }
    
    void DSPRecorder::start() {
        if (m_recording) return;
        if (!m_dsp || !m_masterGroup)
        init();
        if (!m_dsp || !m_masterGroup) {
            log::error("DSPRecorder: failed to initialize");
            return;
        }
        m_masterGroup->addDSP(0, m_dsp);
        {
            auto guard = m_data.lock();
            guard->clear();
        }
        m_recording = true;
        log::info("DSPRecorder: started");
    }
    
    void DSPRecorder::tryUnpause(float time) const {
        if (!m_masterGroup) return;
        auto* system = FMODAudioEngine::sharedEngine()->m_system;
        int sampleRate = 0, channels = 0;
        system->getSoftwareFormat(&sampleRate, nullptr, &channels);
        if (sampleRate <= 0 || channels <= 0) return;
        
        for (int i = 0; i < 16; i++) {
            float capturedTime;
            {
                auto guard = m_data.lock();
                capturedTime = static_cast<float>(guard->size()) /
                (static_cast<float>(sampleRate) * static_cast<float>(channels));
            }
            if (capturedTime >= time) break;
            m_masterGroup->setPaused(false);
            asp::sleep(asp::Duration::fromMillis(1));
        }
    }
    
    void DSPRecorder::stop() {
        if (!m_recording) return;
        m_masterGroup->removeDSP(m_dsp);
        if (m_masterGroup) m_masterGroup->setPaused(false);
        m_recording = false;
        auto guard = m_data.lock();
        log::info("DSPRecorder: stopped, {} samples captured", guard->size());
    }
    
    std::span<float> DSPRecorder::getData() {
        auto guard = m_data.lock();
        return std::span<float>(*guard);
    }
    #endif