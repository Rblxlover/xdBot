#pragma once

#include "gdr/gdr.hpp"
#include "utils/utils.hpp"

#include <gdr/gdr.hpp>
#include <gdr_convert.hpp>

using namespace geode::prelude;

#define DIF(a, b) (std::fabs((a) - (b)) > 0.001f)

const std::vector<float> safeValues = {
    1.0f / 240, 1.0f / 120, 1.0f / 80, 1.0f / 60, 1.0f / 48,
    1.0f / 40, 1.0f / 30, 1.0f / 24, 1.0f / 20, 1.0f / 16,
    1.0f / 15, 1.0f / 12, 1.0f / 10, 1.0f / 8, 1.0f / 6,
    1.0f / 5, 1.0f / 4, 1.0f / 3, 1.0f / 2
};

enum state {
    none,
    recording,
    playing
};

enum class SaveFormat {
    GDR2,
    GDR1,
    JSON
};

struct input : gdr::Input<> {
    input() = default;

    input(uint64_t frame, uint8_t button, bool player2, bool down)
        : Input(frame, button, player2, down) {}

    bool operator==(const input& other) const {
        return frame == other.frame &&
               player2 == other.player2 &&
               button == other.button &&
               down == other.down;
    }

    bool operator<(const input& other) const {
        return frame < other.frame;
    }
};

struct legacy_input : gdr_legacy::Input {
    legacy_input() = default;

    legacy_input(int frame, int button, bool player2, bool down)
        : Input(frame, button, player2, down) {}
};

struct LegacyMacro : gdr_legacy::Replay<LegacyMacro, legacy_input> {
    LegacyMacro() : Replay("xdBot", getModVersionString()) {}
};

struct Macro : gdr::Replay<Macro, input> {

    Macro() : Replay("xdBot", 1) {
        botInfo.name = "xdBot";
    }

public:

    bool canChangeFPS = true;
    uintptr_t seed = 0;
    bool xdBotMacro = true;

    std::vector<gdr_legacy::FrameFix> frameFixes;

    bool shouldParseExtension() const override {
        return botInfo.name == "xdBot";
    }

    void parseExtension(binary_reader& reader) override;
    void saveExtension(binary_writer& writer) const override;

    std::string getBotVersionString() const {
        return getModVersionString();
    }

    LegacyMacro toLegacy() const;

    static Macro fromLegacy(const LegacyMacro& legacy);

    gdr::Result<std::vector<uint8_t>> exportGDR2();

    std::vector<uint8_t> exportGDR1();

    std::vector<uint8_t> exportJSON();

    static Macro importData(std::vector<uint8_t>& data);

    static void recordAction(int frame, int button, bool player2, bool hold);

    static void recordFrameFix(int frame, PlayerObject* p1, PlayerObject* p2);

    static int save(std::string author, std::string desc, std::string path, SaveFormat format = SaveFormat::GDR2);

    static void autoSave(GJGameLevel* level, int number);

    static void tryAutosave(GJGameLevel* level, CheckpointObject* cp);

    static void updateInfo(PlayLayer* pl);

    static void updateTPS();

    static bool loadXDFile(std::filesystem::path path);

    static Macro XDtoGDR(std::filesystem::path path);

    static void resetVariables();

    static void resetState(bool cp = false);

    static void togglePlaying();

    static void toggleRecording();

    static bool shouldStep();

    static bool flipControls();

};

struct button {
    int button;
    bool player2;
    bool down;
};

struct PlayerData {
    std::unordered_map<int, GJPointDouble> m_rotateObjectsRelated;
    std::unordered_map<int, GameObject*> m_potentialSlopeMap;
    std::unordered_set<int> m_touchedRings;
    std::unordered_set<int> m_ringRelatedSet;
    std::map<int, bool> m_jumpPadRelated;
    std::map<int, bool> m_holdingButtons;

    cocos2d::CCPoint position;
    float rotation;

    bool m_holdingRight;
    bool m_holdingLeft;
    bool m_leftPressedFirst;
    bool m_jumpBuffered;
    bool m_stateRingJump;
    bool m_touchedPad;

    double m_yVelocity;
    double m_fallSpeed;
    double m_platformerXVelocity;
    float m_xVelocityRelated;
    float m_xVelocityRelated2;
    float m_platformerVelocityRelated;

    bool m_isDashing;
    DashRingObject* m_dashRing;
    double m_dashX;
    double m_dashY;
    double m_dashAngle;
    double m_dashStartTime;
    double m_lastLandTime;

    float m_rotationSpeed;
    float m_rotateSpeed;
    bool m_isRotating;
    bool m_isBallRotating;
    bool m_isBallRotating2;

    bool m_isAccelerating;
    bool m_affectedByForces;

    bool m_isOnGround;
    bool m_isOnGround2;
    bool m_isOnGround3;
    bool m_isOnGround4;

    bool m_isMoving;
    bool m_platformerMovingLeft;
    bool m_platformerMovingRight;
    bool m_isGoingLeft;

    cocos2d::CCPoint m_lastPortalPos;
    GameObject* m_lastActivatedPortal;
    cocos2d::CCPoint m_lastGroundedPos;
    cocos2d::CCPoint m_shipRotation;
    cocos2d::CCPoint m_lastGroundedPos2;
    cocos2d::CCPoint m_position;

    GameObject* m_currentSlope;
    GameObject* m_currentSlope2;
    GameObject* m_currentSlope3;
    float m_slopeAngle;
    float m_slopeAngleRadians;
    bool m_isOnSlope;
    bool m_wasOnSlope;
    bool m_isCollidingWithSlope;
    bool m_slopeFlipGravityRelated;
    bool m_slopeSlidingMaybeRotated;
    bool m_maybeUpsideDownSlope;
    float m_slopeVelocity;
    double m_slopeRotation;
    double m_currentSlopeYVelocity;
    int m_collidingWithSlopeId;
    double m_maybeSlopeForce;
    bool m_isSliding;
    bool m_isSlidingRight;
    bool m_isOnIce;
    bool m_maybeGoingCorrectSlopeDirection;
    bool m_isCurrentSlopeTop;
    double m_slopeStartTime;
    double m_slopeEndTime;
    double m_maybeSlidingStartTime;
    int m_maybeSlidingTime;

    int m_lastCollisionBottom;
    int m_lastCollisionTop;
    int m_lastCollisionLeft;
    int m_lastCollisionRight;
    double m_collidedTopMinY;
    double m_collidedBottomMaxY;
    double m_collidedLeftMaxX;
    double m_collidedRightMinX;
    GameObject* m_collidedObject;
    GameObject* m_collidingWithLeft;
    GameObject* m_collidingWithRight;
    GameObject* m_lastGroundObject;
    GameObject* m_preLastGroundObject;
    int m_groundObjectMaterial;
    bool m_maybeIsColliding;

    double m_yVelocityRelated;
    double m_yVelocityBeforeSlope;
    float m_yVelocityRelated3;
    double m_groundYVelocity;
    double m_scaleXRelated;
    double m_scaleXRelated2;
    double m_scaleXRelated3;
    double m_scaleXRelated4;
    double m_scaleXRelated5;
    double m_scaleXRelatedTime;
    double m_physDeltaRelated;
    double m_unk3d0;
    double unk_584;

    bool m_isShip;
    bool m_isBird;
    bool m_isBall;
    bool m_isDart;
    bool m_isRobot;
    bool m_isSpider;
    bool m_isSwing;
    bool m_isUpsideDown;
    bool m_isSideways;

    double m_speedMultiplier;
    double m_gravity;
    float m_gravityMod;
    double m_yStart;
    double m_accelerationOrSpeed;
    float m_playerSpeed;
    float m_vehicleSize;
    double m_maybeReverseSpeed;
    double m_maybeReverseAcceleration;
    int m_reverseRelated;

    int m_stateOnGround;
    unsigned char m_stateUnk;
    unsigned char m_stateNoStickX;
    unsigned char m_stateNoStickY;
    unsigned char m_stateUnk2;
    int m_stateBoostX;
    int m_stateBoostY;
    int m_maybeStateForce2;
    int m_stateScale;
    int m_stateNoAutoJump;
    int m_stateDartSlide;
    int m_stateHitHead;
    int m_stateFlipGravity;
    int m_stateForce;
    cocos2d::CCPoint m_stateForceVector;
    unsigned char m_stateJumpBuffered;
    bool m_stateRingJump2;

    bool m_touchedRing;
    bool m_touchedCustomRing;
    bool m_touchedGravityPortal;
    bool m_maybeTouchedBreakableBlock;
    bool m_ringJumpRelated;
    bool m_padRingRelated;
    bool m_wasJumpBuffered;
    bool m_wasRobotJump;
    bool m_canPlaceCheckpoint;
    geode::SeedValueRSV m_jumpRelatedAC2;

    GameObject* m_objectSnappedTo;
    double m_snapDistance;

    double m_gameModeChangedTime;
    double m_lastJumpTime;
    double m_lastFlipTime;
    double m_lastSpiderFlipTime;
    double m_lastCheckpointTime;
    double m_lastLandTime2;
    double m_changedDirectionsTime;
    double m_maybeChangedDirectionAngle;
    double m_blackOrbRelated;
    double m_totalTime;

    bool m_maybeIsFalling;
    bool m_maybeIsBoosted;
    bool m_decreaseBoostSlide;
    bool m_fixGravityBug;
    bool m_reverseSync;
    bool m_wasTeleported;
    bool m_justPlacedStreak;
    bool m_quickCheckpointMode;
    bool m_maybeHasStopped;
    bool m_unk3e0;
    bool m_unk3e1;
    bool m_unk669;
    int m_unk50C;
    int m_unk510;
    int m_unk9e8;
    float m_unk648;
    double m_unkUnused2;
    double m_unkUnused2b;
    bool m_isPlatformer;
    bool m_isLocked;
    bool m_controlsDisabled;
    bool m_inputsLocked;
    bool m_isDead;
    bool m_isSecondPlayer;
    bool m_isBeingSpawnedByDualPortal;
    bool m_isOutOfBounds;
    bool m_hasEverJumped;
    bool m_hasEverHitRing;
    bool m_gv0123;
    bool m_fixRobotJump;
    bool m_unkA29;
    bool m_unkBool5;
    bool m_enable22Changes;
    bool m_ignoreDamage;

    std::vector<float> m_playerFollowFloats;

    int m_maybeSavedPlayerFrame;
    int m_playerStreak;
    int m_followRelated;
    int m_iconRequestID;
    float m_unkAngle1;
    float m_unkUnused3;
    float m_unkAAC;
    float m_somethingPlayerSpeedTime;
    float m_playerSpeedAC;
    float m_trailingParticleLife;
    float m_landParticlesAngle;
    float m_landParticleRelatedY;
    double m_flashTime;
    float m_flashRelated;
    float m_flashRelated1;
    bool m_switchWaveTrailColor;
    bool m_swapColors;
    bool m_defaultMiniIcon;
    bool m_switchDashFireColor;
    bool m_maybeIsVehicleGlowing;
    bool m_hasGlow;
    bool m_isHidden;
    bool m_playEffects;
    bool m_maybeReducedEffects;
    bool m_maybeCanRunIntoBlocks;
    bool m_hasGroundParticles;
    bool m_hasShipParticles;
    bool m_maybeSpriteRelated;
    bool m_useLandParticles0;
    bool m_practiceDeathEffect;
    bool m_shouldTryPlacingCheckpoint;
    bool m_checkpointTimeout;
    bool m_robotAnimation1Enabled;
    bool m_robotAnimation2Enabled;
    bool m_spiderAnimationEnabled;
    bool m_disablePlayerSqueeze;
    int m_unkUnused;
    float m_fallStartY;
    float m_unk838;
    bool m_unk3e0b;
    bool m_unk3e1b;
};

struct CheckpointData {
    int frame;
    PlayerData p1;
    PlayerData p2;
    uint64_t seed;
    int previousFrame;
};