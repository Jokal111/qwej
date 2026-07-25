#pragma once
#include "../SDK/SDK.h"

namespace W2S {

    inline float cachedScreenW = 0.0f;
    inline float cachedScreenH = 0.0f;

    inline void UpdateScreenSize() {
        cachedScreenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        cachedScreenH = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
    }

    inline RBX::Vec2 WorldToScreen(const RBX::Vec3& worldPos, const RBX::Mat4& viewMatrix) {
        RBX::Vec4 quat;

        quat.X = (worldPos.X * viewMatrix.data[0]) + (worldPos.Y * viewMatrix.data[1]) + (worldPos.Z * viewMatrix.data[2]) + viewMatrix.data[3];
        quat.Y = (worldPos.X * viewMatrix.data[4]) + (worldPos.Y * viewMatrix.data[5]) + (worldPos.Z * viewMatrix.data[6]) + viewMatrix.data[7];
        quat.Z = (worldPos.X * viewMatrix.data[8]) + (worldPos.Y * viewMatrix.data[9]) + (worldPos.Z * viewMatrix.data[10]) + viewMatrix.data[11];
        quat.W = (worldPos.X * viewMatrix.data[12]) + (worldPos.Y * viewMatrix.data[13]) + (worldPos.Z * viewMatrix.data[14]) + viewMatrix.data[15];

        RBX::Vec2 screen;

        if (quat.W < 0.1f) {
            return screen;
        }

        RBX::Vec3 ndc;
        ndc.X = quat.X / quat.W;
        ndc.Y = quat.Y / quat.W;
        ndc.Z = quat.Z / quat.W;

        screen.X = (cachedScreenW / 2.0f * ndc.X) + (cachedScreenW / 2.0f);
        screen.Y = -(cachedScreenH / 2.0f * ndc.Y) + (cachedScreenH / 2.0f);

        return screen;
    }
}
