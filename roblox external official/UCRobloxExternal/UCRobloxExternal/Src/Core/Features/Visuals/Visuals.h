#pragma once
#include "../../../Game/W2S/W2S.h"
#include "../../../Core/Cache/Cache.h"
#include "../../../Core/Vars/Vars.h"
#include "../../../Render/ImGui/imgui.h"
#include <string>
#include <algorithm>
#include <cmath>

namespace Visuals {

    inline ImU32 LerpColor(ImU32 c1, ImU32 c2, float t) {
        int r1 = (c1 >> 0) & 0xFF;
        int g1 = (c1 >> 8) & 0xFF;
        int b1 = (c1 >> 16) & 0xFF;
        int a1 = (c1 >> 24) & 0xFF;
        int r2 = (c2 >> 0) & 0xFF;
        int g2 = (c2 >> 8) & 0xFF;
        int b2 = (c2 >> 16) & 0xFF;
        int a2 = (c2 >> 24) & 0xFF;
        return IM_COL32(
            static_cast<int>(r1 + (r2 - r1) * t),
            static_cast<int>(g1 + (g2 - g1) * t),
            static_cast<int>(b1 + (b2 - b1) * t),
            static_cast<int>(a1 + (a2 - a1) * t)
        );
    }

    inline void DrawOutlinedText(ImDrawList* drawList, const ImVec2& pos, const std::string& text, ImU32 textColor) {
        drawList->AddText(ImVec2(pos.x - 1, pos.y), IM_COL32(0, 0, 0, 255), text.c_str());
        drawList->AddText(ImVec2(pos.x + 1, pos.y), IM_COL32(0, 0, 10, 255), text.c_str());
        drawList->AddText(ImVec2(pos.x, pos.y - 1), IM_COL32(0, 0, 0, 255), text.c_str());
        drawList->AddText(ImVec2(pos.x, pos.y + 1), IM_COL32(0, 0, 0, 255), text.c_str());
        drawList->AddText(pos, textColor, text.c_str());
    }

    inline void DrawGradientOutlinedText(ImDrawList* drawList, const ImVec2& pos, const std::string& text, ImU32 colorTop, ImU32 colorBottom) {
        drawList->AddText(ImVec2(pos.x - 1, pos.y), IM_COL32(0, 0, 0, 255), text.c_str());
        drawList->AddText(ImVec2(pos.x + 1, pos.y), IM_COL32(0, 0, 0, 255), text.c_str());
        drawList->AddText(ImVec2(pos.x, pos.y - 1), IM_COL32(0, 0, 0, 255), text.c_str());
        drawList->AddText(ImVec2(pos.x, pos.y + 1), IM_COL32(0, 0, 0, 255), text.c_str());

        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        float midY = pos.y + textSize.y * 0.5f;

        drawList->PushClipRect(ImVec2(pos.x - 2, pos.y - 2), ImVec2(pos.x + textSize.x + 2, midY), true);
        drawList->AddText(pos, colorTop, text.c_str());
        drawList->PopClipRect();

        drawList->PushClipRect(ImVec2(pos.x - 2, midY), ImVec2(pos.x + textSize.x + 2, pos.y + textSize.y + 2), true);
        drawList->AddText(pos, colorBottom, text.c_str());
        drawList->PopClipRect();
    }

    inline RBX::Vec2 Project(const RBX::Vec3& p, const RBX::Mat4& vm) {
        return W2S::WorldToScreen(p, vm);
    }

    inline bool IsValidScreen(const RBX::Vec2& s) {
        return !(s.X == 0.0f && s.Y == 0.0f);
    }

    inline void Draw3DLine(ImDrawList* dl, const RBX::Vec3& a, const RBX::Vec3& b,
        const RBX::Mat4& vm, ImU32 col, float thickness = 1.5f)
    {
        auto sa = Project(a, vm);
        auto sb = Project(b, vm);
        if (!IsValidScreen(sa) || !IsValidScreen(sb)) return;
        dl->AddLine(ImVec2(sa.X, sa.Y), ImVec2(sb.X, sb.Y), col, thickness);
    }

    inline void Draw3DBox(ImDrawList* dl, const RBX::Vec3 corners[8],
        const RBX::Mat4& vm, ImU32 col, float thickness = 1.3f)
    {
        Draw3DLine(dl, corners[0], corners[1], vm, col, thickness);
        Draw3DLine(dl, corners[1], corners[3], vm, col, thickness);
        Draw3DLine(dl, corners[3], corners[2], vm, col, thickness);
        Draw3DLine(dl, corners[2], corners[0], vm, col, thickness);
        Draw3DLine(dl, corners[4], corners[5], vm, col, thickness);
        Draw3DLine(dl, corners[5], corners[7], vm, col, thickness);
        Draw3DLine(dl, corners[7], corners[6], vm, col, thickness);
        Draw3DLine(dl, corners[6], corners[4], vm, col, thickness);
        Draw3DLine(dl, corners[0], corners[4], vm, col, thickness);
        Draw3DLine(dl, corners[1], corners[5], vm, col, thickness);
        Draw3DLine(dl, corners[2], corners[6], vm, col, thickness);
        Draw3DLine(dl, corners[3], corners[7], vm, col, thickness);
    }

    inline void BuildOBBCorners(const RBX::Vec3& center,
        const RBX::Vec3& right, const RBX::Vec3& up, const RBX::Vec3& fwd,
        float hw, float hh, float hd,
        RBX::Vec3 out[8])
    {
        for (int i = 0; i < 8; i++) {
            float sx = (i & 4) ? 1.0f : -1.0f;
            float sy = (i & 2) ? 1.0f : -1.0f;
            float sz = (i & 1) ? 1.0f : -1.0f;
            out[i] = {
                center.X + right.X*sx*hw + up.X*sy*hh + fwd.X*sz*hd,
                center.Y + right.Y*sx*hw + up.Y*sy*hh + fwd.Y*sz*hd,
                center.Z + right.Z*sx*hw + up.Z*sy*hh + fwd.Z*sz*hd
            };
        }
    }

    inline void RenderESP(ImDrawList* drawList, const RBX::Mat4& viewMatrix) 
    {
        if(!Vars::ESP::enabled) return;

        W2S::UpdateScreenSize();

        float screenW = ImGui::GetIO().DisplaySize.x;
        float screenH = ImGui::GetIO().DisplaySize.y;

        for(auto& plr : PlayerCache::players){
            if(!plr.isValid) continue;
            if(Vars::ESP::teamCheck && plr.isTeammate) continue;

            auto character = RBX::RbxInstance(plr.characterAddr);

            // ── Use cached body part addresses from Cache (no re-resolution needed) ──
            auto head = RBX::RbxInstance(plr.headAddr);
            auto torso = RBX::RbxInstance(plr.torsoAddr);
            auto rootPart = RBX::RbxInstance(plr.rootPartAddr);

            // ── Resolve skeleton parts only when skeleton/3D box enabled ──
            bool isR6 = (torso.Addr != 0 && torso.GetName() == "Torso");

            // R6 parts
            auto leftArm  = RBX::RbxInstance(0);
            auto rightArm = RBX::RbxInstance(0);
            auto leftLeg  = RBX::RbxInstance(0);
            auto rightLeg = RBX::RbxInstance(0);

            // R15 parts
            auto upperTorso  = RBX::RbxInstance(0);
            auto lowerTorso  = RBX::RbxInstance(0);
            auto leftUArm    = RBX::RbxInstance(0);
            auto rightUArm   = RBX::RbxInstance(0);
            auto leftLArm    = RBX::RbxInstance(0);
            auto rightLArm   = RBX::RbxInstance(0);
            auto leftHand    = RBX::RbxInstance(0);
            auto rightHand   = RBX::RbxInstance(0);
            auto leftULeg    = RBX::RbxInstance(0);
            auto rightULeg   = RBX::RbxInstance(0);
            auto leftLLeg    = RBX::RbxInstance(0);
            auto rightLLeg   = RBX::RbxInstance(0);
            auto leftFoot    = RBX::RbxInstance(0);
            auto rightFoot   = RBX::RbxInstance(0);

            bool needSkeleton = Vars::ESP::skeleton || Vars::ESP::boxMode == 3;

            if (needSkeleton || Vars::ESP::boxMode == 3) {
                if (isR6) {
                    leftArm  = character.FindChild("Left Arm");
                    rightArm = character.FindChild("Right Arm");
                    leftLeg  = character.FindChild("Left Leg");
                    rightLeg = character.FindChild("Right Leg");
                } else {
                    upperTorso  = character.FindChild("UpperTorso");
                    if (upperTorso.Addr == 0) upperTorso = character.FindChild("Root");
                    lowerTorso  = character.FindChild("LowerTorso");

                    leftUArm  = character.FindChild("LeftUpperArm");
                    rightUArm = character.FindChild("RightUpperArm");
                    leftLArm  = character.FindChild("LeftLowerArm");
                    rightLArm = character.FindChild("RightLowerArm");
                    leftHand  = character.FindChild("LeftHand");
                    rightHand = character.FindChild("RightHand");
                    leftULeg  = character.FindChild("LeftUpperLeg");
                    rightULeg = character.FindChild("RightUpperLeg");
                    leftLLeg  = character.FindChild("LeftLowerLeg");
                    rightLLeg = character.FindChild("RightLowerLeg");
                    leftFoot  = character.FindChild("LeftFoot");
                    rightFoot = character.FindChild("RightFoot");

                    if (!isR6 && upperTorso.Addr == 0 && lowerTorso.Addr == 0) {
                        // Neither R6 nor R15 — try R6 fallbacks
                        isR6 = true;
                        leftArm  = character.FindChild("Left Arm");
                        rightArm = character.FindChild("Right Arm");
                        leftLeg  = character.FindChild("Left Leg");
                        rightLeg = character.FindChild("Right Leg");
                    }
                }
            }

            if (head.Addr == 0) continue;

            // ── Use cached positions from Cache (already computed) ──────────
            RBX::Vec3 headPos = plr.headPosition;
            RBX::Vec3 rootPos = plr.position;

            // Feet position: root.Y - 4.5 (standard Roblox character height)
            RBX::Vec3 feetPos = { rootPos.X, rootPos.Y - 4.5f, rootPos.Z };

            // ── Project head and feet to screen (2 W2S calls, camera-stable)
            auto headScreen = W2S::WorldToScreen(headPos, viewMatrix);
            auto feetScreen = W2S::WorldToScreen(feetPos, viewMatrix);

            // ── Screen-space bounding rect ──────────────────────────────────
            float minX = 999999.0f, minY = 999999.0f;
            float maxX = -999999.0f, maxY = -999999.0f;

            if ((headScreen.X != 0.0f || headScreen.Y != 0.0f) &&
                (feetScreen.X != 0.0f || feetScreen.Y != 0.0f))
            {
                float boxHeight = feetScreen.Y - headScreen.Y;
                float boxWidth = boxHeight * 0.35f;

                float centerX = headScreen.X;
                float topY = headScreen.Y;
                float botY = feetScreen.Y;

                minX = centerX - (boxWidth / 2.0f);
                maxX = centerX + (boxWidth / 2.0f);
                minY = topY;
                maxY = botY;
            }

            if (minX == 999999.0f) {
                if (Vars::ESP::offscreen && head.Addr != 0) {
                    float centerX = screenW / 2.0f;
                    float centerY = screenH / 2.0f;
                    float X = (headPos.X*viewMatrix.data[0]) + (headPos.Y*viewMatrix.data[1]) + (headPos.Z*viewMatrix.data[2]) + viewMatrix.data[3];
                    float Y = (headPos.X*viewMatrix.data[4]) + (headPos.Y*viewMatrix.data[5]) + (headPos.Z*viewMatrix.data[6]) + viewMatrix.data[7];
                    float W = (headPos.X*viewMatrix.data[12]) + (headPos.Y*viewMatrix.data[13]) + (headPos.Z*viewMatrix.data[14]) + viewMatrix.data[15];
                    if (W < 0.1f) {
                        float angle = atan2f(-Y, X);
                        float radius = 150.0f;
                        float indX = centerX + cosf(angle)*radius;
                        float indY = centerY + sinf(angle)*radius;
                        float size = 12.0f;
                        ImVec2 p1 = ImVec2(indX + cosf(angle)*size,        indY + sinf(angle)*size);
                        ImVec2 p2 = ImVec2(indX + cosf(angle+2.5f)*size,   indY + sinf(angle+2.5f)*size);
                        ImVec2 p3 = ImVec2(indX + cosf(angle-2.5f)*size,   indY + sinf(angle-2.5f)*size);
                        drawList->AddTriangleFilled(p1, p2, p3, IM_COL32(255, 50, 50, 255));
                    }
                }
                continue;
            }

            float boxWidth  = maxX - minX;
            float boxHeight = maxY - minY;

            ImVec2 screenSize = ImGui::GetIO().DisplaySize;
            if (boxWidth > screenSize.x*1.5f || boxHeight > screenSize.y*1.5f) continue;

            // ── Filled Box ───────────────────────────────────────────────────
            if (Vars::ESP::filledBox) {
                auto& c = Vars::ESP::filledColor;
                ImU32 fillCol = IM_COL32(
                    (int)(c[0]*255), (int)(c[1]*255),
                    (int)(c[2]*255), (int)(c[3]*255)
                );
                drawList->AddRectFilled(ImVec2(minX, minY), ImVec2(maxX, maxY), fillCol);
            }

            auto& bc = Vars::ESP::boxColor;
            ImU32 boxCol = IM_COL32((int)(bc[0]*255), (int)(bc[1]*255), (int)(bc[2]*255), (int)(bc[3]*255));
            auto& bgc = Vars::ESP::boxGradientColor;
            ImU32 boxGradCol = IM_COL32((int)(bgc[0]*255), (int)(bgc[1]*255), (int)(bgc[2]*255), (int)(bgc[3]*255));
            float thick = Vars::ESP::boxThickness;

            // ── Regular Box ──────────────────────────────────────────────────
            if (Vars::ESP::boxMode == 1) {
                drawList->AddRect(ImVec2(minX - 1, minY - 1), ImVec2(maxX + 1, maxY + 1), IM_COL32(0,0,0,220), 0.0f, 0, 1.0f);
                drawList->AddRect(ImVec2(minX + thick, minY + thick), ImVec2(maxX - thick, maxY - thick), IM_COL32(0,0,0,220), 0.0f, 0, 1.0f);

                if (Vars::ESP::boxGradient) {
                    drawList->AddRectFilledMultiColor(ImVec2(minX, minY), ImVec2(minX + thick, maxY), boxCol, boxCol, boxGradCol, boxGradCol);
                    drawList->AddRectFilledMultiColor(ImVec2(maxX - thick, minY), ImVec2(maxX, maxY), boxCol, boxCol, boxGradCol, boxGradCol);
                    drawList->AddRectFilled(ImVec2(minX + thick, minY), ImVec2(maxX - thick, minY + thick), boxCol);
                    drawList->AddRectFilled(ImVec2(minX + thick, maxY - thick), ImVec2(maxX - thick, maxY), boxGradCol);
                } else {
                    drawList->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), boxCol, 0.0f, 0, thick);
                }
            }

            // ── Corner Box ───────────────────────────────────────────────────
            if (Vars::ESP::boxMode == 2) {
                float lineW = (maxX - minX) / 4.0f;
                float lineH = (maxY - minY) / 4.0f;
                auto DrawCorner = [&](ImVec2 a, ImVec2 b, ImVec2 c, ImVec2 d, ImU32 col, float t) {
                    drawList->AddLine(a, b, col, t); drawList->AddLine(c, d, col, t);
                };
                
                DrawCorner(ImVec2(minX,minY), ImVec2(minX+lineW,minY), ImVec2(minX,minY), ImVec2(minX,minY+lineH), IM_COL32(0,0,0,220), thick + 2.0f);
                DrawCorner(ImVec2(maxX,minY), ImVec2(maxX-lineW,minY), ImVec2(maxX,minY), ImVec2(maxX,minY+lineH), IM_COL32(0,0,0,220), thick + 2.0f);
                DrawCorner(ImVec2(minX,maxY), ImVec2(minX+lineW,maxY), ImVec2(minX,maxY), ImVec2(minX,maxY-lineH), IM_COL32(0,0,0,220), thick + 2.0f);
                DrawCorner(ImVec2(maxX,maxY), ImVec2(maxX-lineW,maxY), ImVec2(maxX,maxY), ImVec2(maxX,maxY-lineH), IM_COL32(0,0,0,220), thick + 2.0f);

                if (Vars::ESP::boxGradient) {
                    ImU32 colTL_End = LerpColor(boxCol, boxGradCol, lineH / boxHeight);
                    ImU32 colBL_Start = LerpColor(boxCol, boxGradCol, (boxHeight - lineH) / boxHeight);
                    
                    drawList->AddRectFilled(ImVec2(minX, minY), ImVec2(minX + lineW, minY + thick), boxCol);
                    drawList->AddRectFilledMultiColor(ImVec2(minX, minY), ImVec2(minX + thick, minY + lineH), boxCol, boxCol, colTL_End, colTL_End);
                    
                    drawList->AddRectFilled(ImVec2(maxX - lineW, minY), ImVec2(maxX, minY + thick), boxCol);
                    drawList->AddRectFilledMultiColor(ImVec2(maxX - thick, minY), ImVec2(maxX, minY + lineH), boxCol, boxCol, colTL_End, colTL_End);
                    
                    drawList->AddRectFilled(ImVec2(minX, maxY - thick), ImVec2(minX + lineW, maxY), boxGradCol);
                    drawList->AddRectFilledMultiColor(ImVec2(minX, maxY - lineH), ImVec2(minX + thick, maxY), colBL_Start, colBL_Start, boxGradCol, boxGradCol);
                    
                    drawList->AddRectFilled(ImVec2(maxX - lineW, maxY - thick), ImVec2(maxX, maxY), boxGradCol);
                    drawList->AddRectFilledMultiColor(ImVec2(maxX - thick, maxY - lineH), ImVec2(maxX, maxY), colBL_Start, colBL_Start, boxGradCol, boxGradCol);
                } else {
                    DrawCorner(ImVec2(minX,minY), ImVec2(minX+lineW,minY), ImVec2(minX,minY), ImVec2(minX,minY+lineH), boxCol, thick);
                    DrawCorner(ImVec2(maxX,minY), ImVec2(maxX-lineW,minY), ImVec2(maxX,minY), ImVec2(maxX,minY+lineH), boxCol, thick);
                    DrawCorner(ImVec2(minX,maxY), ImVec2(minX+lineW,maxY), ImVec2(minX,maxY), ImVec2(minX,maxY-lineH), boxCol, thick);
                    DrawCorner(ImVec2(maxX,maxY), ImVec2(maxX-lineW,maxY), ImVec2(maxX,maxY), ImVec2(maxX,maxY-lineH), boxCol, thick);
                }
            }

            // ── 3D Box ───────────────────────────────────────────────────────
            if (Vars::ESP::boxMode == 3 && torso.Addr != 0) {
                auto BuildAndDraw3D = [&](RBX::RbxInstance& inst, float w, float h, float d, ImU32 col) {
                    if (inst.Addr == 0) return;
                    auto rot = inst.GetRotation();
                    auto pos = rot.GetPosition();
                    auto r   = rot.GetRightVector();
                    auto u   = rot.GetUpVector();
                    auto f   = rot.GetLookVector();
                    RBX::Vec3 corners[8];
                    BuildOBBCorners(pos, r, u, f, w*0.5f, h*0.5f, d*0.5f, corners);
                    Draw3DBox(drawList, corners, viewMatrix, col, 1.2f);
                };

                ImU32 col3D = IM_COL32(120, 220, 255, 220);
                if (isR6) {
                    BuildAndDraw3D(torso,    2.0f, 2.0f, 1.0f, col3D);
                    BuildAndDraw3D(head,     1.0f, 1.0f, 1.0f, col3D);
                    BuildAndDraw3D(rightArm, 1.0f, 2.0f, 1.0f, col3D);
                    BuildAndDraw3D(leftArm,  1.0f, 2.0f, 1.0f, col3D);
                    BuildAndDraw3D(rightLeg, 1.0f, 2.0f, 1.0f, col3D);
                    BuildAndDraw3D(leftLeg,  1.0f, 2.0f, 1.0f, col3D);
                } else {
                    BuildAndDraw3D(head,       1.0f, 1.0f, 1.0f, col3D);
                    BuildAndDraw3D(upperTorso, 1.6f, 1.5f, 0.8f, col3D);
                    BuildAndDraw3D(lowerTorso, 1.4f, 1.0f, 0.8f, col3D);
                    BuildAndDraw3D(leftUArm,   0.6f, 1.0f, 0.6f, col3D);
                    BuildAndDraw3D(rightUArm,  0.6f, 1.0f, 0.6f, col3D);
                    BuildAndDraw3D(leftLArm,   0.5f, 1.0f, 0.5f, col3D);
                    BuildAndDraw3D(rightLArm,  0.5f, 1.0f, 0.5f, col3D);
                    BuildAndDraw3D(leftULeg,   0.6f, 1.0f, 0.6f, col3D);
                    BuildAndDraw3D(rightULeg,  0.6f, 1.0f, 0.6f, col3D);
                    BuildAndDraw3D(leftLLeg,   0.5f, 0.8f, 0.5f, col3D);
                    BuildAndDraw3D(rightLLeg,  0.5f, 0.8f, 0.5f, col3D);
                }
            }

            // ── Skeleton ─────────────────────────────────────────────────────
            if (Vars::ESP::skeleton) {
                auto& skc = Vars::ESP::skeletonColor;
                ImU32 skelCol = IM_COL32((int)(skc[0]*255), (int)(skc[1]*255), (int)(skc[2]*255), (int)(skc[3]*255));
                ImU32 skelShadow = IM_COL32(0, 0, 0, 180);
                float skelThick = Vars::ESP::skeletonThickness;

                auto DrawBone = [&](RBX::RbxInstance& a, RBX::RbxInstance& b) {
                    if (a.Addr == 0 || b.Addr == 0) return;
                    auto sa = Project(a.GetPos(), viewMatrix);
                    auto sb = Project(b.GetPos(), viewMatrix);
                    if (!IsValidScreen(sa) || !IsValidScreen(sb)) return;
                    drawList->AddLine(ImVec2(sa.X, sa.Y), ImVec2(sb.X, sb.Y), skelShadow, skelThick + 1.5f);
                    drawList->AddLine(ImVec2(sa.X, sa.Y), ImVec2(sb.X, sb.Y), skelCol, skelThick);
                };

                auto DrawDot = [&](RBX::RbxInstance& inst, float r) {
                    if (inst.Addr == 0) return;
                    auto p = Project(inst.GetPos(), viewMatrix);
                    if (!IsValidScreen(p)) return;
                    drawList->AddCircleFilled(ImVec2(p.X, p.Y), r + 1.0f, skelShadow);
                    drawList->AddCircleFilled(ImVec2(p.X, p.Y), r, skelCol);
                };

                if (isR6) {
                    DrawBone(head, torso);
                    DrawBone(torso, rightArm);
                    DrawBone(torso, leftArm);
                    DrawBone(torso, rightLeg);
                    DrawBone(torso, leftLeg);
                    DrawDot(head, 3.5f);
                    DrawDot(torso, 2.5f);
                    DrawDot(rightArm, 2.0f);
                    DrawDot(leftArm, 2.0f);
                    DrawDot(rightLeg, 2.0f);
                    DrawDot(leftLeg, 2.0f);
                } else {
                    DrawBone(head, upperTorso);
                    DrawBone(upperTorso, lowerTorso);
                    DrawBone(upperTorso, leftUArm);
                    DrawBone(upperTorso, rightUArm);
                    DrawBone(leftUArm, leftLArm);
                    DrawBone(rightUArm, rightLArm);
                    DrawBone(leftLArm, leftHand);
                    DrawBone(rightLArm, rightHand);
                    DrawBone(lowerTorso, leftULeg);
                    DrawBone(lowerTorso, rightULeg);
                    DrawBone(leftULeg, leftLLeg);
                    DrawBone(rightULeg, rightLLeg);
                    DrawBone(leftLLeg, leftFoot);
                    DrawBone(rightLLeg, rightFoot);

                    DrawDot(head, 3.5f);
                    DrawDot(upperTorso, 2.5f);
                    DrawDot(lowerTorso, 2.0f);
                    DrawDot(leftUArm, 2.0f); DrawDot(rightUArm, 2.0f);
                    DrawDot(leftLArm, 2.0f); DrawDot(rightLArm, 2.0f);
                    DrawDot(leftHand, 2.0f); DrawDot(rightHand, 2.0f);
                    DrawDot(leftULeg, 2.0f); DrawDot(rightULeg, 2.0f);
                    DrawDot(leftLLeg, 2.0f); DrawDot(rightLLeg, 2.0f);
                    DrawDot(leftFoot, 2.0f); DrawDot(rightFoot, 2.0f);
                }
            }

            // ── Health Bar ───────────────────────────────────────────────────
            if (Vars::ESP::healthBar && plr.maxHealth > 0) {
                float hp = static_cast<float>(plr.health) / static_cast<float>(plr.maxHealth);
                ImU32 healthColor = IM_COL32((int)(255*(1-hp)), (int)(255*hp), 0, 255);
                float barH = (maxY - minY) * hp;
                drawList->AddRectFilled(ImVec2(minX-6, minY), ImVec2(minX-2, maxY), IM_COL32(0,0,0,200));
                drawList->AddRectFilled(ImVec2(minX-5, maxY-barH), ImVec2(minX-3, maxY), healthColor);
            }

            // ── Health Text ──────────────────────────────────────────────────
            if (Vars::ESP::healthText && plr.maxHealth > 0) {
                float hp = static_cast<float>(plr.health) / static_cast<float>(plr.maxHealth);
                std::string hpText = std::to_string(static_cast<int>(plr.health)) + " HP";
                ImVec2 ts = ImGui::CalcTextSize(hpText.c_str());
                float hpY = maxY - ((maxY-minY)*hp) - (ts.y/2.0f);
                if (hpY < minY - ts.y) hpY = minY - ts.y;
                if (hpY > maxY) hpY = maxY;
                float leftOffset = Vars::ESP::healthBar ? 10.0f : 4.0f;
                ImU32 hcol = IM_COL32((int)(255*(1-hp)), (int)(255*hp), 0, 255);
                ImVec2 textPos = ImVec2(minX - leftOffset - ts.x, hpY);
                if (Vars::ESP::textGradient) {
                    auto& tgc = Vars::ESP::textGradientColor;
                    ImU32 tgCol = IM_COL32((int)(tgc[0]*255), (int)(tgc[1]*255), (int)(tgc[2]*255), (int)(tgc[3]*255));
                    DrawGradientOutlinedText(drawList, textPos, hpText, hcol, tgCol);
                } else {
                    DrawOutlinedText(drawList, textPos, hpText, hcol);
                }
            }

            // ── Names ────────────────────────────────────────────────────────
            if (Vars::ESP::nameMode != 0) {
                std::string drawName = (Vars::ESP::nameMode == 2 && !plr.displayName.empty()) ? plr.displayName : plr.name;
                ImVec2 ts = ImGui::CalcTextSize(drawName.c_str());
                auto& nc = Vars::ESP::nameColor;
                ImU32 nCol = IM_COL32((int)(nc[0]*255), (int)(nc[1]*255), (int)(nc[2]*255), (int)(nc[3]*255));
                ImVec2 textPos = ImVec2((minX+maxX)/2.0f - ts.x/2.0f, minY - ts.y - 2);
                if (Vars::ESP::textGradient) {
                    auto& tgc = Vars::ESP::textGradientColor;
                    ImU32 tgCol = IM_COL32((int)(tgc[0]*255), (int)(tgc[1]*255), (int)(tgc[2]*255), (int)(tgc[3]*255));
                    DrawGradientOutlinedText(drawList, textPos, drawName, nCol, tgCol);
                } else {
                    DrawOutlinedText(drawList, textPos, drawName, nCol);
                }
            }

            // ── Distance ─────────────────────────────────────────────────────
            if (Vars::ESP::distance) {
                std::string distText = std::to_string(static_cast<int>(plr.distance)) + "m";
                ImVec2 ts = ImGui::CalcTextSize(distText.c_str());
                auto& dc = Vars::ESP::distanceColor;
                ImU32 dCol = IM_COL32((int)(dc[0]*255), (int)(dc[1]*255), (int)(dc[2]*255), (int)(dc[3]*255));
                ImVec2 textPos = ImVec2((minX+maxX)/2.0f - ts.x/2.0f, maxY + 2);
                if (Vars::ESP::textGradient) {
                    auto& tgc = Vars::ESP::textGradientColor;
                    ImU32 tgCol = IM_COL32((int)(tgc[0]*255), (int)(tgc[1]*255), (int)(tgc[2]*255), (int)(tgc[3]*255));
                    DrawGradientOutlinedText(drawList, textPos, distText, dCol, tgCol);
                } else {
                    DrawOutlinedText(drawList, textPos, distText, dCol);
                }
            }

            // ── Head Dot ───────────────────────────────────────────────────────
            if (Vars::ESP::headDot && head.Addr != 0) {
                RBX::Vec2 headScreen = W2S::WorldToScreen(headPos, viewMatrix);
                if (IsValidScreen(headScreen)) {
                    auto& hdc = Vars::ESP::headDotColor;
                    ImU32 hdCol = IM_COL32((int)(hdc[0]*255), (int)(hdc[1]*255), (int)(hdc[2]*255), (int)(hdc[3]*255));
                    drawList->AddCircleFilled(ImVec2(headScreen.X, headScreen.Y), 4.0f, hdCol);
                    drawList->AddCircle(ImVec2(headScreen.X, headScreen.Y), 5.0f, IM_COL32(0, 0, 0, 255), 12, 1.5f);
                }
            }

            // ── View Angle ─────────────────────────────────────────────────────
            if (Vars::ESP::viewAngle && head.Addr != 0) {
                RBX::Vec3 lookVec = head.GetRotation().GetLookVector();
                RBX::Vec3 endPos = {headPos.X - lookVec.X * 5.0f, headPos.Y - lookVec.Y * 5.0f, headPos.Z - lookVec.Z * 5.0f};
                
                RBX::Vec2 headScreen = W2S::WorldToScreen(headPos, viewMatrix);
                RBX::Vec2 endScreen = W2S::WorldToScreen(endPos, viewMatrix);
                
                if (IsValidScreen(headScreen) && IsValidScreen(endScreen)) {
                    drawList->AddLine(ImVec2(headScreen.X, headScreen.Y), ImVec2(endScreen.X, endScreen.Y), IM_COL32(255, 50, 50, 255), 2.0f);
                }
            }

            // ── Tracers ──────────────────────────────────────────────────────
            if (Vars::ESP::tracers) {
                ImVec2 origin = ImVec2(screenW/2.0f, screenH);
                if (Vars::ESP::tracerOrigin == 0) origin = ImVec2(screenW/2.0f, 0.0f);
                else if (Vars::ESP::tracerOrigin == 1) origin = ImVec2(screenW/2.0f, screenH/2.0f);
                else if (Vars::ESP::tracerOrigin == 3) {
                    POINT p; GetCursorPos(&p);
                    origin = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
                }
                auto& tc = Vars::ESP::tracerColor;
                ImU32 tCol = IM_COL32((int)(tc[0]*255), (int)(tc[1]*255), (int)(tc[2]*255), (int)(tc[3]*255));
                drawList->AddLine(origin, ImVec2((minX+maxX)/2.0f, maxY), tCol, 1.0f);
            }
        }
    }
}
