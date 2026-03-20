// practice_fixes.hpp
#pragma once
#include "../includes.hpp"
#include "../macro.hpp"
#include "checkpoint.hpp"
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class PracticeFix {
    public:
    static bool shouldEnable() {
        PlayLayer* pl = PlayLayer::get();
        if (!pl || !pl->m_isPracticeMode) return false;
        
        auto& g = Global::get();
        if (g.state != state::none) return true;
        
        if (g.alwaysPracticeFixes && pl->m_isPlatformer) return true;
        
        return false;
    }
};

class PlayerPracticeFixes {
    public:
    struct SavedState {
        SupplementalPlayerState supplemental;
        double m_dashX = 0.0, m_dashY = 0.0, m_dashAngle = 0.0, m_dashStartTime = 0.0;
        double m_slopeStartTime = 0.0;
        bool m_justPlacedStreak = false;
        std::vector<float> m_playerFollowFloats;
        double m_fallSpeed = 0.0;
        float m_xVelocityRelated = 0.f, m_xVelocityRelated2 = 0.f;
        double m_scaleXRelated = 0.0, m_scaleXRelated2 = 0.0, m_scaleXRelated3 = 0.0;
        double m_scaleXRelated4 = 0.0, m_scaleXRelated5 = 0.0;
        double m_groundYVelocity = 0.0, m_yVelocityRelated = 0.0;
        double m_physDeltaRelated = 0.0;
        double m_gravity = 0.0;
        float m_gravityMod = 0.f;
        double m_speedMultiplier = 0.0;
        float m_playerSpeed = 0.f, m_vehicleSize = 0.f;
        double m_accelerationOrSpeed = 0.0;
        float m_platformerVelocityRelated = 0.f;
    };
    
    static SavedState saveData(PlayerObject* p) {
        SavedState s;
        if (!p) return s;
        s.supplemental            = SupplementalPlayerState(p);
        s.m_dashX                 = p->m_dashX;
        s.m_dashY                 = p->m_dashY;
        s.m_dashAngle             = p->m_dashAngle;
        s.m_dashStartTime         = p->m_dashStartTime;
        s.m_slopeStartTime        = p->m_slopeStartTime;
        s.m_justPlacedStreak      = p->m_justPlacedStreak;
        s.m_playerFollowFloats    = p->m_playerFollowFloats;
        s.m_fallSpeed             = p->m_fallSpeed;
        s.m_xVelocityRelated      = p->m_xVelocityRelated;
        s.m_xVelocityRelated2     = p->m_xVelocityRelated2;
        s.m_scaleXRelated         = p->m_scaleXRelated;
        s.m_scaleXRelated2        = p->m_scaleXRelated2;
        s.m_scaleXRelated3        = p->m_scaleXRelated3;
        s.m_scaleXRelated4        = p->m_scaleXRelated4;
        s.m_scaleXRelated5        = p->m_scaleXRelated5;
        s.m_groundYVelocity       = p->m_groundYVelocity;
        s.m_yVelocityRelated      = p->m_yVelocityRelated;
        s.m_physDeltaRelated      = p->m_physDeltaRelated;
        s.m_gravity               = p->m_gravity;
        s.m_gravityMod            = p->m_gravityMod;
        s.m_speedMultiplier       = p->m_speedMultiplier;
        s.m_playerSpeed           = p->m_playerSpeed;
        s.m_vehicleSize           = p->m_vehicleSize;
        s.m_accelerationOrSpeed   = p->m_accelerationOrSpeed;
        s.m_platformerVelocityRelated = p->m_platformerVelocityRelated;
        return s;
    }
    
    static void applyData(PlayerObject* p, const SavedState& s) {
        if (!p) return;
        s.supplemental.apply(p);
        p->m_dashX                    = s.m_dashX;
        p->m_dashY                    = s.m_dashY;
        p->m_dashAngle                = s.m_dashAngle;
        p->m_dashStartTime            = s.m_dashStartTime;
        p->m_slopeStartTime           = s.m_slopeStartTime;
        p->m_justPlacedStreak         = s.m_justPlacedStreak;
        p->m_playerFollowFloats       = s.m_playerFollowFloats;
        p->m_fallSpeed                = s.m_fallSpeed;
        p->m_xVelocityRelated         = s.m_xVelocityRelated;
        p->m_xVelocityRelated2        = s.m_xVelocityRelated2;
        p->m_scaleXRelated            = s.m_scaleXRelated;
        p->m_scaleXRelated2           = s.m_scaleXRelated2;
        p->m_scaleXRelated3           = s.m_scaleXRelated3;
        p->m_scaleXRelated4           = s.m_scaleXRelated4;
        p->m_scaleXRelated5           = s.m_scaleXRelated5;
        p->m_groundYVelocity          = s.m_groundYVelocity;
        p->m_yVelocityRelated         = s.m_yVelocityRelated;
        p->m_physDeltaRelated         = s.m_physDeltaRelated;
        p->m_gravity                  = s.m_gravity;
        p->m_gravityMod               = s.m_gravityMod;
        p->m_speedMultiplier          = s.m_speedMultiplier;
        p->m_playerSpeed              = s.m_playerSpeed;
        p->m_vehicleSize              = s.m_vehicleSize;
        p->m_accelerationOrSpeed      = s.m_accelerationOrSpeed;
        p->m_platformerVelocityRelated = s.m_platformerVelocityRelated;
    }
    
    static void transfer(PlayerObject* from, PlayerObject* to, bool applyPos) {
        if (!from || !to) return;
        SavedState s = saveData(from);
        if (applyPos) {
            to->setPosition(from->getPosition());
            to->setRotation(from->getRotation());
        }
        applyData(to, s);
    }
};