#pragma once
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/PlayerCheckpoint.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/DashRingObject.hpp>

using namespace geode::prelude;

struct SupplementalPlayerState {
    double m_yVelocity = 0.0;
    double m_platformerXVelocity = 0.0;
    bool m_isOnGround = false;
    cocos2d::CCPoint m_lastPortalPos = {};
    GameObject* m_lastActivatedPortal = nullptr;
    cocos2d::CCPoint m_lastGroundedPos = {};
    bool m_isDashing = false;
    DashRingObject* m_dashRing = nullptr;
    double m_lastLandTime = 0.0;
    bool m_isAccelerating = false;
    bool m_affectedByForces = false;
    float m_rotationSpeed = 0.0f;
    bool m_isRotating = false;
    bool m_isBallRotating = false;
    bool m_isBallRotating2 = false;
    bool m_jumpBuffered = false;
    bool m_stateRingJump = false;
    bool m_touchedPad = false;
    bool m_isMoving = false;
    bool m_holdingRight = false;
    bool m_holdingLeft = false;
    bool m_leftPressedFirst = false;
    std::map<int, bool> m_holdingButtons;
    std::unordered_map<int, GJPointDouble> m_rotateObjectsRelated;
    std::unordered_map<int, GameObject*> m_potentialSlopeMap;

    SupplementalPlayerState() = default;

    SupplementalPlayerState(PlayerObject* p) {
        if (!p) return;
        m_yVelocity             = p->m_yVelocity;
        m_platformerXVelocity   = p->m_platformerXVelocity;
        m_isOnGround            = p->m_isOnGround;
        m_lastPortalPos         = p->m_lastPortalPos;
        m_lastActivatedPortal   = p->m_lastActivatedPortal;
        m_lastGroundedPos       = p->m_lastGroundedPos;
        m_isDashing             = p->m_isDashing;
        m_dashRing              = p->m_dashRing;
        m_lastLandTime          = p->m_lastLandTime;
        m_isAccelerating        = p->m_isAccelerating;
        m_affectedByForces      = p->m_affectedByForces;
        m_rotationSpeed         = p->m_rotationSpeed;
        m_isRotating            = p->m_isRotating;
        m_isBallRotating        = p->m_isBallRotating;
        m_isBallRotating2       = p->m_isBallRotating2;
        m_jumpBuffered          = p->m_jumpBuffered;
        m_stateRingJump         = p->m_stateRingJump;
        m_touchedPad            = p->m_touchedPad;
        m_isMoving              = p->m_isMoving;
        m_holdingRight          = p->m_holdingRight;
        m_holdingLeft           = p->m_holdingLeft;
        m_leftPressedFirst      = p->m_leftPressedFirst;
        m_holdingButtons        = p->m_holdingButtons;
        m_rotateObjectsRelated  = p->m_rotateObjectsRelated;
        m_potentialSlopeMap = p->m_potentialSlopeMap;
    }

    void apply(PlayerObject* p) const {
        if (!p) return;
        p->m_yVelocity            = m_yVelocity;
        p->m_platformerXVelocity  = m_platformerXVelocity;
        p->m_isOnGround           = m_isOnGround;
        p->m_lastPortalPos        = m_lastPortalPos;
        p->m_lastActivatedPortal  = m_lastActivatedPortal;
        p->m_lastGroundedPos      = m_lastGroundedPos;
        p->m_isDashing            = m_isDashing;
        p->m_dashRing             = m_dashRing;
        p->m_lastLandTime         = m_lastLandTime;
        p->m_isAccelerating       = m_isAccelerating;
        p->m_affectedByForces     = m_affectedByForces;
        p->m_rotationSpeed        = m_rotationSpeed;
        p->m_isRotating           = m_isRotating;
        p->m_isBallRotating       = m_isBallRotating;
        p->m_isBallRotating2      = m_isBallRotating2;
        p->m_jumpBuffered         = m_jumpBuffered;
        p->m_stateRingJump        = m_stateRingJump;
        p->m_touchedPad           = m_touchedPad;
        p->m_isMoving             = m_isMoving;
        p->m_holdingRight         = m_holdingRight;
        p->m_holdingLeft          = m_holdingLeft;
        p->m_leftPressedFirst     = m_leftPressedFirst;
        p->m_holdingButtons       = m_holdingButtons;
        p->m_rotateObjectsRelated = m_rotateObjectsRelated;
        p->m_potentialSlopeMap = m_potentialSlopeMap;
    }
};

struct SupplementalPlayLayerState {
    GameObject* m_player1CollisionBlock = nullptr;
    GameObject* m_player2CollisionBlock = nullptr;
    double m_extraDelta = 0.0;
    int m_currentStep = 0;
    float m_unk3380 = 0.0f;

    SupplementalPlayLayerState() = default;

    SupplementalPlayLayerState(PlayLayer* pl) {
        if (!pl) return;
        m_player1CollisionBlock = pl->m_player1CollisionBlock;
        m_player2CollisionBlock = pl->m_player2CollisionBlock;
        m_extraDelta            = pl->m_extraDelta;
        m_currentStep           = pl->m_currentStep;
        m_unk3380               = pl->m_unk3380;
    }

    void apply(PlayLayer* pl) const {
        if (!pl) return;
        pl->m_player1CollisionBlock = m_player1CollisionBlock;
        pl->m_player2CollisionBlock = m_player2CollisionBlock;
        pl->m_extraDelta            = m_extraDelta;
        pl->m_currentStep           = m_currentStep;
        pl->m_unk3380               = m_unk3380;
    }
};