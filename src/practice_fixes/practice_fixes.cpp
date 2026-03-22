#include "checkpoint.hpp"
#include "practice_fixes.hpp"
#include <Geode/modify/PlayLayer.hpp>

void resetTPSBypassState();

struct PracticeCheckpointData {
    SupplementalPlayerState    p1, p2;
    SupplementalPlayLayerState pl;
    
    PracticeCheckpointData() = default;
    PracticeCheckpointData(PlayerObject* p1Obj, PlayerObject* p2Obj, PlayLayer* plObj) {
        if (!plObj || !p1Obj) return;
        pl = SupplementalPlayLayerState(plObj);
        p1 = SupplementalPlayerState(p1Obj);
        if (p2Obj) p2 = SupplementalPlayerState(p2Obj);
    }
    
    void apply(PlayerObject* p1Obj, PlayerObject* p2Obj, PlayLayer* plObj) const {
        if (!plObj) return;
        pl.apply(plObj);
        if (p1Obj) p1.apply(p1Obj);
        if (p2Obj) p2.apply(p2Obj);
    }
};

class $modify(FixPlayLayer, PlayLayer) {
    struct Fields {
        std::unordered_map<CheckpointObject*, PracticeCheckpointData> m_checkpoints;
        std::unordered_map<CheckpointObject*, std::vector<input>> m_checkpointInputs;
        std::unordered_map<CheckpointObject*, std::vector<gdr_legacy::FrameFix>> m_checkpointFrameFixes;
        std::unordered_map<CheckpointObject*, int> m_checkpointFrames;
    };
    
    void loadFromCheckpoint(CheckpointObject* checkpoint) {
        bool shouldFix = PracticeFix::shouldEnable();
        
        if (shouldFix) {
            if (m_player1) m_player1->m_isDashing = false;
            if (m_gameState.m_isDualMode && m_player2) m_player2->m_isDashing = false;
        }
        
        PlayLayer::loadFromCheckpoint(checkpoint);
        
        resetTPSBypassState();
        
        auto* fields = m_fields.self();
        
        if (shouldFix) {
            auto it = fields->m_checkpoints.find(checkpoint);
            if (it != fields->m_checkpoints.end()) {
                it->second.apply(
                    m_player1,
                    m_gameState.m_isDualMode ? m_player2 : nullptr,
                    this
                );
            }
        }
        
        auto& g = Global::get();
        // Only restore inputs from checkpoint during recording mode
        // During playback mode, don't replace the macro inputs when loading a checkpoint
        if (g.state == state::recording) {
            auto inputIt = fields->m_checkpointInputs.find(checkpoint);
            if (inputIt != fields->m_checkpointInputs.end()) {
                g.ignoreRecordAction = true;
                g.macro.inputs = inputIt->second;
                auto fixIt = fields->m_checkpointFrameFixes.find(checkpoint);
                if (fixIt != fields->m_checkpointFrameFixes.end())
                    g.macro.frameFixes = fixIt->second;
                
                auto frameIt = fields->m_checkpointFrames.find(checkpoint);
                if (frameIt != fields->m_checkpointFrames.end()) {
                    g.m_frameCount = frameIt->second;
                    int targetFrame = g.m_frameCount - g.frameOffset;
                    g.currentAction = 0;
                    while (g.currentAction < g.macro.inputs.size() && g.macro.inputs[g.currentAction].frame < targetFrame) {
                        g.currentAction++;
                    }
                    g.currentFrameFix = 0;
                    while (g.currentFrameFix < g.macro.frameFixes.size() && g.macro.frameFixes[g.currentFrameFix].frame < targetFrame) {
                        g.currentFrameFix++;
                    }
                }
                g.ignoreRecordAction = false;
            }
        }
    }
    
    CheckpointObject* createCheckpoint() {
        auto* checkpoint = PlayLayer::createCheckpoint();
        if (!checkpoint) return checkpoint;
        
        bool shouldFix = PracticeFix::shouldEnable();
        if (shouldFix) {
            auto* fields = m_fields.self();
            fields->m_checkpoints[checkpoint] = PracticeCheckpointData(
                m_player1,
                m_gameState.m_isDualMode ? m_player2 : nullptr,
                this
            );
        }
        
        auto& g = Global::get();
        if (g.state == state::recording || g.state == state::playing) {
            auto* fields = m_fields.self();
            g.ignoreRecordAction = true;
            fields->m_checkpointInputs[checkpoint]     = g.macro.inputs;
            fields->m_checkpointFrameFixes[checkpoint] = g.macro.frameFixes;
            fields->m_checkpointFrames[checkpoint]     = Global::getCurrentFrame();
            g.ignoreRecordAction = false;
        }
        
        return checkpoint;
    }
    
    void removeCheckpoint(CheckpointObject* checkpoint) {
        PlayLayer::removeCheckpoint(checkpoint);
        
        auto* fields = m_fields.self();
        fields->m_checkpoints.erase(checkpoint);
        fields->m_checkpointInputs.erase(checkpoint);
        fields->m_checkpointFrameFixes.erase(checkpoint);
        fields->m_checkpointFrames.erase(checkpoint);
    }
    
    void resetLevel() {
        bool hadCheckpoints = m_checkpointArray->count() > 0;
        
        if (!hadCheckpoints) {
            auto* fields = m_fields.self();
            fields->m_checkpoints.clear();
            fields->m_checkpointInputs.clear();
            fields->m_checkpointFrameFixes.clear();
            fields->m_checkpointFrames.clear();
        }
        
        PlayLayer::resetLevel();
        
        if (hadCheckpoints) m_resumeTimer = 0;
    }
};