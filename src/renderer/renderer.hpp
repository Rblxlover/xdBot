#pragma once

#include "../includes.hpp"
#include "ffmpeg/events.hpp"

#ifndef GEODE_IS_IOS

#include <atomic>
#include <thread>

class RendererSpinlock {
public:
    void wait_for(bool state) const {
        while (m_flag.load(std::memory_order_acquire) != state)
            std::this_thread::yield();
    }
    [[nodiscard]] bool read() const { return m_flag.load(std::memory_order_acquire); }
    void set(bool state) { m_flag.store(state, std::memory_order_release); }
private:
    std::atomic<bool> m_flag { false };
};

enum class AudioMode {
    Off    = 0,
    Record = 1
};

class xdBotRenderTexture {
public:
    unsigned width = 0, height = 0;
    int old_fbo = 0, old_rbo = 0;
    unsigned fbo = 0;
    GLuint texture = 0;
    void begin();
    void end();
    void capture(cocos2d::CCNode* node, std::vector<uint8_t>& buffer, RendererSpinlock& frameReady);
};

class Renderer {
public:
    Renderer() : width(1920), height(1080), fps(60) {}

    bool levelFinished   = false;
    bool recording       = false;
    bool isPlatformer    = false;
    AudioMode audioMode  = AudioMode::Off;
    float SFXVolume      = 1.f;
    float musicVolume    = 1.f;

    bool usingApi        = false;
    bool dontRender      = false;
    int  levelStartFrame = 0;

    float    stopAfter   = 3.f;
    float    timeAfter   = 0.f;
    unsigned width, height;
    unsigned fps;
    double   lastFrame_t = 0.0;
    double   extra_t     = 0.0;

    ffmpeg::events::Recorder ffmpeg;

#ifdef GEODE_IS_WINDOWS
    std::string ffmpegPath = geode::utils::string::pathToString(
        geode::dirs::getGameDir() / "ffmpeg.exe");
#endif

    std::string codec, bitrate, extraArgs, videoArgs, extraAudioArgs, path;
    std::unordered_set<int> renderedFrames;

    FMODAudioEngine* fmod     = nullptr;
    cocos2d::CCSize  ogRes    = {0, 0};
    float            ogScaleX = 1.f;
    float            ogScaleY = 1.f;

    void captureFrame();
    void changeRes(bool og);

    void start();
    void stop(int frame = 0);
    void handleRecording(PlayLayer* pl, int frame);

    static bool toggle();
    static bool shouldUseAPI();
    bool tryPause() { return true; }

private:
    RendererSpinlock     m_frameReady;
    std::vector<uint8_t> m_currentFrame;
    xdBotRenderTexture      m_renderTexture;

    void recordThread();
    void showEndScreenIfNeeded();
};

#endif