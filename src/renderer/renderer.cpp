#include "../includes.hpp"
#include "../ui/game_ui.hpp"
#include "DSPRecorder.hpp"

#include <Geode/modify/CCCircleWave.hpp>
#include <Geode/modify/CCParticleSystemQuad.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#ifdef GEODE_IS_WINDOWS
#include "../utils/subprocess.hpp"
#endif

#include <filesystem>
#include <thread>

#ifndef GEODE_IS_IOS

static bool writePCMWav(const std::filesystem::path& outPath,
                        std::span<const float> pcm, FMOD::System* system);

class $modify(CCParticleSystemQuad) {
    static CCParticleSystemQuad* create(const char* v1, bool v2) {
        CCParticleSystemQuad* ret = CCParticleSystemQuad::create(v1, v2);
        if (!Global::get().renderer.recording) return ret;
        if (std::string_view(v1) == "levelComplete01.plist" &&
            Mod::get()->getSavedValue<bool>("render_hide_levelcomplete"))
            ret->setVisible(false);
        return ret;
    }
};

class $modify(CCCircleWave) {
    static CCCircleWave* create(float v1, float v2, float v3, bool v4, bool v5) {
        CCCircleWave* ret = CCCircleWave::create(v1, v2, v3, v4, v5);
        if (!Global::get().renderer.recording ||
            !PlayLayer::get()->m_levelEndAnimationStarted) return ret;
        if (Mod::get()->getSavedValue<bool>("render_hide_levelcomplete"))
            ret->setVisible(false);
        return ret;
    }
};

class $modify(RenderPlayLayerHook, PlayLayer) {
    void showCompleteText() {
        PlayLayer::showCompleteText();
        if (!Global::get().renderer.recording) return;
        if (m_levelEndAnimationStarted &&
            Mod::get()->getSavedValue<bool>("render_hide_levelcomplete")) {
            for (CCNode* node : CCArrayExt<CCNode*>(getChildren())) {
                CCSprite* spr = typeinfo_cast<CCSprite*>(node);
                if (!spr) continue;
                if (!isSpriteFrameName(spr, "GJ_levelComplete_001.png")) continue;
                spr->setVisible(false);
            }
        }
    }
};

class $modify(EndLevelLayer) {
    void customSetup() {
        EndLevelLayer::customSetup();
        if (!PlayLayer::get()) return;
        if (Global::get().renderer.recording &&
            PlayLayer::get()->m_levelEndAnimationStarted &&
            Mod::get()->getSavedValue<bool>("render_hide_endscreen")) {
            Loader::get()->queueInMainThread([this] { setVisible(false); });
        }
    }
};

class $modify(FMODAudioEngine) {
    int playEffect(gd::string path, float speed, float p2, float volume) {
        if (path == "explode_11.ogg" && Global::get().renderer.recording) return 0;
        return FMODAudioEngine::playEffect(path, speed, p2, volume);
    }
};

class $modify(GJBaseGameLayer) {
    void update(float dt) {
        GJBaseGameLayer::update(dt);
        auto& g = Global::get();
        PlayLayer* pl = PlayLayer::get();
        if (!g.renderer.recording || !pl) return;
        int frame  = Global::getCurrentFrame();
        int tpsInt = static_cast<int>(Global::getTPS());
        int fpsInt = static_cast<int>(g.renderer.fps);
        if (tpsInt <= 0 || fpsInt <= 0) return;
        if (frame % (tpsInt / fpsInt) == 0)
            g.renderer.handleRecording(pl, frame);
    }
};

static float schedulerLeftOver = 0.f;

class $modify(CCScheduler) {
    void update(float dt) {
        Renderer& r = Global::get().renderer;
        if (!r.recording) return CCScheduler::update(dt);
        r.changeRes(false);
        float newDt     = 1.f / Global::getTPS();
        auto  startTime = asp::time::Instant::now();
        int   mult      = static_cast<int>((dt + schedulerLeftOver) / newDt);
        for (int i = 0; i < mult; ++i) {
            CCScheduler::update(newDt);
            if (startTime.elapsed() > asp::time::Duration::fromMillis(33)) {
                mult = i + 1;
                break;
            }
        }
        schedulerLeftOver += (dt - newDt * mult);
        r.changeRes(true);
    }
};

bool Renderer::shouldUseAPI() {
    return Loader::get()->isModLoaded("eclipse.ffmpeg-api");
}

bool Renderer::toggle() {
    auto& g = Global::get();

    if (Loader::get()->isModLoaded("syzzi.click_between_frames")) {
        FLAlertLayer::create("Render", "Disable CBF in Geode to render a level.", "OK")->show();
        return false;
    }

    if (GameManager::sharedState()->getGameVariable(GameVar::ClickBetweenSteps)) {
        auto scene = CCScene::get();
        if (scene && !scene->getChildByID("render-alert"_spr)) {
            auto alert = FLAlertLayer::create(
                "Render", "Disable <cr>Click Between Steps</c> to render a level.", "OK");
            alert->setID("render-alert"_spr);
            alert->show();
        }
        return false;
    }

    bool foundApi = shouldUseAPI();

#ifdef GEODE_IS_WINDOWS
    std::filesystem::path ffmpegSettingPath =
        Mod::get()->getSettingValue<std::filesystem::path>("ffmpeg_path");
    bool foundExe = std::filesystem::exists(ffmpegSettingPath) &&
        geode::utils::string::pathToString(ffmpegSettingPath.filename()) == "ffmpeg.exe";
#endif

    if (g.renderer.recording) {
        g.renderer.stop(Global::getCurrentFrame());
    } else {
#ifdef GEODE_IS_WINDOWS
        if (!foundApi && !foundExe) {
            geode::createQuickPopup("Error",
                "<cl>FFmpeg</c> not found. Install eclipse.ffmpeg-api, or set the path "
                "to ffmpeg.exe in mod settings.\nOpen download link?",
                "Cancel", "Yes", [](auto, bool btn2) {
                    if (btn2) {
                        FLAlertLayer::create("Info",
                            "Unzip the downloaded file and look for <cl>ffmpeg.exe</c> "
                            "in the 'bin' folder.", "OK")->show();
                        utils::web::openLinkInBrowser(
                            "https://www.gyan.dev/ffmpeg/builds/ffmpeg-git-essentials.7z");
                    }
                });
            return false;
        }
        if (!foundApi)
            g.renderer.ffmpegPath = geode::utils::string::pathToString(ffmpegSettingPath);
#else
        if (!foundApi) {
            FLAlertLayer::create("Error",
                "The <cl>eclipse.ffmpeg-api</c> mod is required for rendering on this platform.",
                "OK")->show();
            return false;
        }
#endif

        if (!PlayLayer::get()) {
            FLAlertLayer::create("Warning", "<cl>Open a level</c> to start rendering it.", "OK")->show();
            return false;
        }

#ifdef GEODE_IS_IOS
        std::filesystem::path renderFolder = Mod::get()->getSaveDir() / "renders";
#else
        std::filesystem::path renderFolder =
            Mod::get()->getSettingValue<std::filesystem::path>("render_folder");
#endif

        if (!std::filesystem::exists(renderFolder)) {
            if (!utils::file::createDirectoryAll(renderFolder).isOk()) {
                FLAlertLayer::create("Error",
                    "There was an error getting the renders folder. ID: 11", "OK")->show();
                return false;
            }
        }

        g.renderer.usingApi = foundApi;
        g.renderer.start();
    }

    Interface::updateLabels();
    return true;
}

void Renderer::start() {
    PlayLayer* pl  = PlayLayer::get();
    Mod*       mod = Mod::get();
    fmod = FMODAudioEngine::sharedEngine();

    fps          = geode::utils::numFromString<int>(
                       mod->getSavedValue<std::string>("render_fps")).unwrapOr(60);
    codec        = mod->getSavedValue<std::string>("render_codec");
    bitrate      = mod->getSavedValue<std::string>("render_bitrate") + "M";
    extraArgs    = mod->getSavedValue<std::string>("render_args");
    videoArgs    = mod->getSavedValue<std::string>("render_video_args");
    extraAudioArgs = mod->getSavedValue<std::string>("render_audio_args");
    SFXVolume    = mod->getSavedValue<double>("render_sfx_volume");
    musicVolume  = mod->getSavedValue<double>("render_music_volume");
    stopAfter    = geode::utils::numFromString<float>(
                       mod->getSavedValue<std::string>("render_seconds_after")).unwrapOr(0.f);
    audioMode    = AudioMode::Record;

    std::string extension = mod->getSavedValue<std::string>("render_file_extension");
    auto timestamp = asp::time::SystemTime::now().timeSinceEpoch().millis();
    std::string filename = fmt::format("render_{}_{}{}",
        std::string_view(pl->m_level->m_levelName),
        geode::utils::numToString(timestamp), extension);

#ifdef GEODE_IS_IOS
    path = geode::utils::string::pathToString(mod->getSaveDir() / "renders" / filename);
#else
    path = geode::utils::string::pathToString(
        mod->getSettingValue<std::filesystem::path>("render_folder") / filename);
#endif

    width  = geode::utils::numFromString<int>(
                 mod->getSavedValue<std::string>("render_width2")).unwrapOr(1920);
    height = geode::utils::numFromString<int>(
                 mod->getSavedValue<std::string>("render_height")).unwrapOr(1080);
    if (width  % 2 != 0) width++;
    if (height % 2 != 0) height++;

    m_renderTexture = MyRenderTexture();
    m_renderTexture.width  = width;
    m_renderTexture.height = height;
    ogRes    = cocos2d::CCEGLView::get()->getDesignResolutionSize();
    ogScaleX = cocos2d::CCEGLView::get()->m_fScaleX;
    ogScaleY = cocos2d::CCEGLView::get()->m_fScaleY;

    recording     = true;
    levelFinished = false;
    timeAfter     = 0.f;
    lastFrame_t   = extra_t = 0.0;
    dontRender    = true;

    renderedFrames.clear();
    m_currentFrame.resize(width * height * 4, 0);

    m_renderTexture.begin();
    changeRes(false);

    DSPRecorder::get()->start();

    if (!pl->m_levelEndAnimationStarted && pl->m_isPaused)
        Global::get().restart = true;

    if (Global::get().state != state::playing && !Global::get().macro.inputs.empty())
        Macro::togglePlaying();

    if (!mod->setSavedValue("first_render_", true)) {
        FLAlertLayer::create("Warning",
            "If you have a macro for the level, <cl>let it run</c> to allow the level to render.",
            "OK")->show();
    }

    async::runtime().spawnBlocking<void>([this]() { recordThread(); });
}

void Renderer::recordThread() {
    geode::utils::thread::setName("xdBot Recorder Thread");

    Mod* mod = Mod::get();

    int bitrateApi = geode::utils::numFromString<int>(
        mod->getSavedValue<std::string>("render_bitrate")).unwrapOr(12) * 1000000;

    ffmpeg::RenderSettings settings;
    settings.m_pixelFormat       = ffmpeg::PixelFormat::RGBA;
    settings.m_codec             = codec;
    settings.m_bitrate           = bitrateApi;
    settings.m_width             = width;
    settings.m_height            = height;
    settings.m_fps               = static_cast<uint16_t>(fps);
    settings.m_outputFile        = path;
    settings.m_colorspaceFilters = videoArgs;
    settings.m_doVerticalFlip    = true;

#ifdef GEODE_IS_WINDOWS
    subprocess::Popen process;
#endif

    if (usingApi) {
        auto res = ffmpeg.init(settings);
        if (res.isErr()) {
            recording = false;
            m_frameReady.set(true);
            Loader::get()->queueInMainThread([] {
                FLAlertLayer::create("Error", "FFmpeg API failed to initialize.", "OK")->show();
            });
            return;
        }
    }
#ifdef GEODE_IS_WINDOWS
    else {
        std::string c  = codec.empty()   ? "" : ("-c:v " + codec + " ");
        std::string b  = bitrate.empty() ? "" : ("-b:v " + bitrate + " ");
        std::string ea = extraArgs.empty() ? "-pix_fmt yuv420p" : extraArgs;
        std::string va = videoArgs.empty()
            ? "colorspace=all=bt709:iall=bt470bg:fast=1" : videoArgs;

        float fadeInTime = geode::utils::numFromString<float>(
            mod->getSavedValue<std::string>("render_fade_in_time")).unwrapOr(0.f);
        bool fadeInVideo = mod->getSavedValue<bool>("render_fade_in") && fadeInTime != 0.f;
        std::string fadeArgs;
        if (fadeInVideo)
            fadeArgs = fmt::format(",fade=t=in:st=0:d={}", fadeInTime);

        std::string command = fmt::format(
        "\"{}\" -y -f rawvideo -pix_fmt rgba -s {}x{} -r {} -i - {}{}{} "
        "-vf \"vflip,{}{}\" -an \"{}\" ",
            ffmpegPath,
            geode::utils::numToString(width), geode::utils::numToString(height),
            geode::utils::numToString(fps),
            c, b, ea, va, fadeArgs, path);

        log::info("Renderer: {}", command);
        process = subprocess::Popen(command);
    }
#endif

    m_frameReady.set(false);
    m_frameReady.wait_for(true);

    while (recording) {
        if (usingApi) {
            auto res = ffmpeg.writeFrame(m_currentFrame);
            if (res.isErr()) {
                Loader::get()->queueInMainThread([] {
                    FLAlertLayer::create("Error", "FFmpeg API failed to write frame.", "OK")->show();
                });
                stop();
                break;
            }
        }
#ifdef GEODE_IS_WINDOWS
        else {
            process.m_stdin.write(m_currentFrame.data(), m_currentFrame.size());
        }
#endif

        if (!recording) break;

        m_frameReady.set(false);
        m_frameReady.wait_for(true);
    }

    log::debug("Renderer: record thread stopped.");

    if (usingApi) {
        ffmpeg.stop();
    }
#ifdef GEODE_IS_WINDOWS
    else {
        if (process.close()) {
            Loader::get()->queueInMainThread([] {
                FLAlertLayer::create("Error",
                    "There was an error saving the render. Wrong render Args.", "OK")->show();
            });
            return;
        }
    }
#endif

    auto pcm = DSPRecorder::get()->getData();
    
    Loader::get()->queueInMainThread([] {
        Notification::create("Saving Render...", NotificationIcon::Loading)->show();
    });
    asp::sleep(asp::Duration::fromMillis(100));

    if (audioMode == AudioMode::Off || pcm.empty() ||
        (SFXVolume == 0.f && musicVolume == 0.f)) {
        Loader::get()->queueInMainThread([this] {
            Notification::create("Render Saved Without Audio", NotificationIcon::Success)->show();
            showEndScreenIfNeeded();
        });
        return;
    }

    std::filesystem::path videoPath = path;
    std::filesystem::path tempPath  = videoPath.parent_path() /
        ("temp_" + geode::utils::string::pathToString(videoPath.filename()));

    int sampleRate = 48000, audioChannels = 2;
    if (fmod && fmod->m_system)
        fmod->m_system->getSoftwareFormat(&sampleRate, nullptr, &audioChannels);
    double capturedLastFrame = lastFrame_t;
    size_t expectedSamples = static_cast<size_t>(capturedLastFrame * sampleRate * audioChannels);

    log::info("Renderer: pcm={} expected={} lastFrame_t={:.3f}s",
        pcm.size(), expectedSamples, capturedLastFrame);

    std::span<float> pcmSpan = std::span<float>(pcm);
if (pcmSpan.size() > expectedSamples)
    pcmSpan = pcmSpan.subspan(0, expectedSamples);
    if (pcmSpan.size() > expectedSamples)
        pcmSpan = pcmSpan.subspan(0, expectedSamples);

    log::info("Renderer: mixing audio, pcm samples={}, video duration={:.3f}s",
        pcmSpan.size(), capturedLastFrame);

    geode::Result<> mixRes = geode::Err("not started");

    if (usingApi) {
        mixRes = ffmpeg::events::AudioMixer::mixVideoRaw(videoPath, pcmSpan, tempPath);
    }
#ifdef GEODE_IS_WINDOWS
    else {
        std::filesystem::path tempWav = Mod::get()->getSaveDir() / "temp_audio_file.wav";
        if (!writePCMWav(tempWav, pcmSpan, fmod ? fmod->m_system : nullptr)) {
            Loader::get()->queueInMainThread([] {
                FLAlertLayer::create("Error", "Failed to write captured audio to WAV.", "OK")->show();
            });
            return;
        }

        double totalTime = capturedLastFrame;
        if (!extraAudioArgs.empty()) extraAudioArgs += " ";

        std::string cmd = fmt::format(
            "\"{}\" -y -i \"{}\" -i \"{}\" -t {} -c:v copy {}"
            "-filter:a \"[1:a]adelay=0|0\" \"{}\"",
            ffmpegPath,
            geode::utils::string::pathToString(tempWav),
            path, totalTime, extraAudioArgs,
            geode::utils::string::pathToString(tempPath));

        log::info("Renderer (recorded audio): {}", cmd);
        auto proc = subprocess::Popen(cmd);
        if (proc.close()) mixRes = geode::Err(std::string("Wrong Audio Args"));
        else              mixRes = geode::Ok();

        std::error_code ec;
        std::filesystem::remove(tempWav, ec);
    }
#endif

    if (mixRes.isErr()) {
        log::error("Renderer: mix failed: {}", mixRes.unwrapErr());
        Loader::get()->queueInMainThread([err = mixRes.unwrapErr()] {
            FLAlertLayer::create("Error",
                fmt::format("Failed to mix audio: {}", err).c_str(), "OK")->show();
        });
        Loader::get()->queueInMainThread([this] { showEndScreenIfNeeded(); });
        return;
    }

    std::error_code ec;
    std::filesystem::remove(videoPath, ec);
    if (ec) {
        log::warn("Renderer: failed to remove original: {}", ec.message());
    } else {
        ec.clear();
        std::filesystem::rename(tempPath, videoPath, ec);
        if (ec) log::warn("Renderer: failed to rename output: {}", ec.message());
    }

    Loader::get()->queueInMainThread([this] {
        Notification::create("Render Saved With Audio", NotificationIcon::Success)->show();
        showEndScreenIfNeeded();
    });
}

void Renderer::stop(int /*frame*/) {
    if (!recording) return;
    recording = false;

    m_frameReady.set(true);

    m_renderTexture.end();
    changeRes(true);
    DSPRecorder::get()->stop();
}

void Renderer::changeRes(bool og) {
    cocos2d::CCEGLView* view = cocos2d::CCEGLView::get();
    cocos2d::CCSize res;
    float scaleX, scaleY;

    if (og) {
        res    = ogRes;
        scaleX = ogScaleX;
        scaleY = ogScaleY;
    } else {
        res    = CCSize(320.f * (width / static_cast<float>(height)), 320.f);
        scaleX = width  / res.width;
        scaleY = height / res.height;
    }

    if (res == CCSize(0, 0) && !og) return changeRes(true);

    CCDirector::sharedDirector()->m_obWinSizeInPoints = res;
    view->setDesignResolutionSize(res.width, res.height, ResolutionPolicy::kResolutionExactFit);
    view->m_fScaleX = scaleX;
    view->m_fScaleY = scaleY;
}

void MyRenderTexture::begin() {
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);

    auto* tex = new cocos2d::CCTexture2D();
    {
        std::unique_ptr<char, void(*)(void*)> data(
            static_cast<char*>(malloc(width * height * 4)), free);
        memset(data.get(), 0, width * height * 4);
        tex->initWithData(data.get(), cocos2d::kCCTexture2DPixelFormat_RGBA8888,
            width, height,
            cocos2d::CCSize(static_cast<float>(width), static_cast<float>(height)));
    }
    texture = tex;

    glGetIntegerv(GL_RENDERBUFFER_BINDING, &old_rbo);
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, texture->getName(), 0);
    texture->setAliasTexParameters();
    texture->autorelease();
    glBindRenderbuffer(GL_RENDERBUFFER, old_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
}

void MyRenderTexture::end() {
    if (texture) { /* autorelease handles cleanup */ texture = nullptr; }
    if (fbo) { glDeleteFramebuffers(1, &fbo); fbo = 0; }
}

void MyRenderTexture::capture(cocos2d::CCNode* node, std::vector<uint8_t>& buffer,
                               RendererSpinlock& frameReady) {
    auto* director = cocos2d::CCDirector::sharedDirector();

    glViewport(0, 0, width, height);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &old_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    node->visit();

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

    frameReady.set(true);

    glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
    director->setViewport();
}

void Renderer::captureFrame() {
    m_frameReady.wait_for(false);

    if (!recording) return;

    m_renderTexture.capture(PlayLayer::get(), m_currentFrame, m_frameReady);
}

void Renderer::handleRecording(PlayLayer* pl, int frame) {
    if (!pl) { stop(frame); return; }
    isPlatformer = pl->m_isPlatformer;
    if (dontRender || pl->m_player1->m_isDead) return;

    if (renderedFrames.contains(frame) && frame > 10) return;
    renderedFrames.insert(frame);

    if (!pl->m_hasCompletedLevel || timeAfter < stopAfter) {
        float dt = 1.f / static_cast<double>(fps);
        if (pl->m_hasCompletedLevel) {
            timeAfter    += dt;
            levelFinished = true;
        }

        float time = pl->m_gameState.m_levelTime + extra_t - lastFrame_t;
        if (time >= dt) {
            extra_t     = time - dt;
            lastFrame_t = pl->m_gameState.m_levelTime;

            captureFrame();

            DSPRecorder::get()->tryUnpause(static_cast<float>(lastFrame_t));

            fmod->m_globalChannel->setVolume(SFXVolume);
            fmod->m_backgroundMusicChannel->setVolume(musicVolume);
        }
    } else {
        stop(frame);
    }
}

static bool writePCMWav(const std::filesystem::path& outPath,
                        std::span<const float> pcm, FMOD::System* system) {
    int sampleRate = 48000, channels = 2;
    if (system) system->getSoftwareFormat(&sampleRate, nullptr, &channels);

    uint32_t dataSize      = static_cast<uint32_t>(pcm.size() * sizeof(float));
    uint32_t byteRate      = sampleRate * channels * sizeof(float);
    uint16_t blockAlign    = static_cast<uint16_t>(channels * sizeof(float));
    uint16_t bitsPerSample = 32;
    uint16_t audioFmt      = 3;
    uint16_t ch16          = static_cast<uint16_t>(channels);
    uint32_t sr32          = static_cast<uint32_t>(sampleRate);
    uint32_t riffSize      = 36 + dataSize;
    uint32_t fmtSize       = 16;

    std::string data;
    data.append("RIFF", 4);
    data.append(reinterpret_cast<const char*>(&riffSize),      4);
    data.append("WAVE", 4);
    data.append("fmt ", 4);
    data.append(reinterpret_cast<const char*>(&fmtSize),       4);
    data.append(reinterpret_cast<const char*>(&audioFmt),      2);
    data.append(reinterpret_cast<const char*>(&ch16),          2);
    data.append(reinterpret_cast<const char*>(&sr32),          4);
    data.append(reinterpret_cast<const char*>(&byteRate),      4);
    data.append(reinterpret_cast<const char*>(&blockAlign),    2);
    data.append(reinterpret_cast<const char*>(&bitsPerSample), 2);
    data.append("data", 4);
    data.append(reinterpret_cast<const char*>(&dataSize),      4);
    data.append(reinterpret_cast<const char*>(pcm.data()), dataSize);

    auto span = std::span(reinterpret_cast<const unsigned char*>(data.data()), data.size());
    return geode::utils::file::writeBinary(outPath, span).isOk();
}

void Renderer::showEndScreenIfNeeded() {
    if (!Mod::get()->getSavedValue<bool>("render_hide_endscreen")) return;
    if (PlayLayer* pl = PlayLayer::get())
        if (EndLevelLayer* layer = pl->getChildByType<EndLevelLayer>(0))
            layer->setVisible(true);
}

#endif