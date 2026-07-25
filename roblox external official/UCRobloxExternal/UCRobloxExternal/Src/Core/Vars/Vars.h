#pragma once

namespace Vars {
    inline bool menuOpen = false;
    inline int selectedTab = 0;

    namespace Explorer {
        inline bool enabled = false;
    }

    namespace Aimbot {
        inline bool enabled = false;
        inline bool showFOV = false;
        inline float fovRadius = 100.0f;
        inline float smoothing = 5.0f;
        inline int aimTarget = 0;
        inline int aimMethod = 0;
        inline int aimbotKey = 2; // Default Right Mouse
        inline bool enableCameraAimbot = false;

        inline bool teamCheck = false;
        inline bool downedCheck = true;
        inline bool invisCheck = false;
        inline bool invincibleCheck = false;
        
        inline bool prediction = false;
        inline float predX = 0.0f;
        inline float predY = 0.0f;

        inline bool silentAim = false;
        inline bool showSilentFOV = false;
        inline float silentFOV = 100.0f;
        inline int silentAimbotKey = 2;
        inline bool silentPrediction = false;
        inline bool randomTarget = false;
    }

    namespace Triggerbot {
        inline bool enabled = false;
        inline int  key = 2;
        inline bool hitboxes[4] = { true, true, false, false }; // 0:Head, 1:Torso, 2:Arms, 3:Legs
        inline float hitbox_scale = 1.0f;
        inline float delay = 0.0f;
        inline float interval = 80.0f;
        inline float fov = 40.0f;
        inline bool teamCheck = false;
    }

    namespace ESP {
        inline bool enabled = false;
        inline int boxMode = 1; // 0 = Off, 1 = 2D Box, 2 = Corner Box, 3 = 3D Box
        inline int nameMode = 1; // 0 = Off, 1 = Username, 2 = Display Name
        inline bool distance = false;
        inline bool healthBar = false;
        inline bool healthText = false;
        inline bool tracers = false;
        inline int tracerOrigin = 2; // 0=Top, 1=Center, 2=Bottom, 3=Mouse
        inline bool offscreen = false;

        inline bool skeleton = false;
        inline bool filledBox = false;
        inline float filledColor[4] = { 1.0f, 0.2f, 0.2f, 0.25f }; // RGBA
        inline bool headDot = false;
        inline bool viewAngle = false;
        inline bool teamCheck = false;

        inline bool staticBox = false;
        inline float boxColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        inline float boxThickness = 1.0f;
        inline bool boxGradient = false;
        inline float boxGradientColor[4] = { 0.45f, 0.30f, 0.85f, 1.0f };
        inline bool textGradient = false;
        inline float textGradientColor[4] = { 0.45f, 0.30f, 0.85f, 1.0f };
        inline float skeletonColor[4] = { 1.0f, 1.0f, 1.0f, 0.9f };
        inline float skeletonThickness = 1.5f;
        inline float nameColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        inline float distanceColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        inline float tracerColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        inline float headDotColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        
        inline bool crosshair = false;
        inline float crosshairSize = 10.0f;
        inline float crosshairThickness = 1.0f;
        inline float crosshairGap = 2.0f;
        inline bool crosshairDot = false;
        inline float crosshairColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    }

    namespace Local {
        inline bool speedEnabled = false;
        inline float walkSpeed = 16.0f;
        inline bool jumpEnabled = false;
        inline float jumpPower = 50.0f;
        inline bool flightEnabled = false;
        inline float flightSpeed = 50.0f;
        inline int flightKey = 0x70; // F1 to toggle flight
        inline bool infiniteJumpEnabled = false;
        inline bool desyncEnabled = false;
        inline int desyncKey = 0x71; // F2 to toggle desync
        inline int desyncType = 0;
        inline int serverMode = 0;
        inline int clientMode = 0;
        inline bool noclipEnabled = false;
        inline int noclipKey = 0x72; // F3 to toggle noclip
    }

    namespace Misc {
        inline bool streamProof = false;
        inline bool fastLaunch = false;
        inline bool rescan = false;
        inline bool autoRescan = true;
    }

    namespace World {
        inline bool skybox = false;
        inline int skyboxType = 0;
        inline bool rotateSkybox = false;
        inline float rotateSpeed = 0.5f;

        inline bool ambience = false;
        inline float ambienceColor[3] = {1.0f, 1.0f, 1.0f};

        inline bool fog = false;
        inline float fogDistance = 1000.0f;
        inline float fogColor[3] = {1.0f, 1.0f, 1.0f};

        inline bool brightness = false;
        inline float brightnessValue = 1.0f;

        inline bool exposure = false;
        inline float exposureValue = 1.0f;

        inline bool fov = false;
        inline float fovValue = 70.0f;
    }

    namespace Rage {
        inline bool antiAim = false;
    }
}
