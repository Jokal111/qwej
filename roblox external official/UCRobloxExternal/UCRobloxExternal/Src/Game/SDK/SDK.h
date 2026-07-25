#pragma once
#include "../Offsets/Offsets.h"
#include "../../Memory/Communication.h"
#include <string>
#include <vector>
#include <cmath>

namespace RBX {

    struct Vec2 {
        float X{ 0 };
        float Y{ 0 };
    };

    struct Vec3 {
        float X{ 0 };
        float Y{ 0 };
        float Z{ 0 };
    };

    struct Vec4 {
        float X{ 0 };
        float Y{ 0 };
        float Z{ 0 };
        float W{ 0 };
    };

    struct Mat3 {
        float data[9];
    };

    struct Mat4 {
        float data[16];
    };

    struct Rotation {
        float data[12];

        Vec3 GetRightVector() const {
            return { data[0], data[3], data[6] };
        }

        Vec3 GetUpVector() const {
            return { data[1], data[4], data[7] };
        }

        Vec3 GetLookVector() const {
            return { data[2], data[5], data[8] };
        }

        Vec3 GetPosition() const {
            return { data[9], data[10], data[11] };
        }
    };

    class RbxInstance {
    public:
        uintptr_t Addr;

        RbxInstance(uintptr_t addr) : Addr(addr) {}

        std::string GetName() {
            uintptr_t namePtr = Coms->ReadMemory<uintptr_t>(Addr + Offsets::Instance::Name);
            if (namePtr == 0) return "";
            
            return Coms->ReadGameString(namePtr);
        }

        std::string GetClass() {
            uintptr_t classDesc = Coms->ReadMemory<uintptr_t>(Addr + Offsets::Instance::ClassDescriptor);
            if (classDesc == 0) return "";

            
            uintptr_t namePtr = Coms->ReadMemory<uintptr_t>(classDesc + Offsets::Instance::ClassName);
            if (namePtr == 0) return "";
            
            return Coms->ReadGameString(namePtr);
        }

        RbxInstance GetParent() {
            return RbxInstance(Coms->ReadMemory<uintptr_t>(Addr + Offsets::Instance::Parent));
        }


        std::vector<RbxInstance> GetChildList() {
            std::vector<RbxInstance> childList;
            
            uintptr_t childStart = Coms->ReadMemory<uintptr_t>(Addr + Offsets::Instance::ChildrenStart);
            if (childStart == 0) return childList;

            uintptr_t childEnd = Coms->ReadMemory<uintptr_t>(childStart + Offsets::Instance::ChildrenEnd);
            uintptr_t current = Coms->ReadMemory<uintptr_t>(childStart);

            if (current == 0 || childEnd <= current) return childList;

            size_t count = (childEnd - current) / 0x10;
            // Cap at 10,000 children to prevent memory exhaustion and abort() crashes
            if (count > 10000) count = 10000;

            childList.reserve(count);

            for (size_t i = 0; i < count; ++i) {
                uintptr_t childAddr = Coms->ReadMemory<uintptr_t>(current + i * 0x10);
                if (childAddr != 0) {
                    childList.emplace_back(childAddr);
                }
            }
            
            return childList;
        }

        RbxInstance FindChild(const std::string& targetName) {
            auto children = GetChildList();

            
            for (auto& child : children) {
                if (child.GetName() == targetName) {
                    return child;


                }
            }
            
            return RbxInstance(0);

        }

        RbxInstance FindChildByClass(const std::string& targetClass) {
            auto children = GetChildList();
            
            for (auto& child : children) {

                if (child.GetClass() == targetClass) {
                    return child;
                }
            }
            
            return RbxInstance(0);
        }



        RbxInstance WaitChild(const std::string& targetName) {
            while (true) {
                auto children = GetChildList();
                
                for (auto& child : children) {
                    if (child.GetName() == targetName) {
                        return child;
                    }
                }
                Sleep(50);
            }
        }

        RbxInstance FindCharacterPart(const std::string& targetName) {
            auto part = FindChild(targetName);
            if (part.Addr != 0) return part;

            // Try common name alternatives (fewer traversals than full chain)
            if (targetName == "HumanoidRootPart") {
                part = FindChild("Root");
                if (part.Addr != 0) return part;
            }
            else if (targetName == "UpperTorso" || targetName == "Torso") {
                part = FindChild("Chest");
                if (part.Addr != 0) return part;
                part = FindChild("UpperTorso");
                if (part.Addr != 0) return part;
                part = FindChild("Root");
                if (part.Addr != 0) return part;
            }
            else if (targetName == "LowerTorso") {
                part = FindChild("Abdomen");
                if (part.Addr != 0) return part;
            }
            else if (targetName == "LeftArm" || targetName == "LeftHand" || targetName == "LeftUpperArm") {
                part = FindChild("LeftArm");
                if (part.Addr != 0) return part;
            }
            else if (targetName == "RightArm" || targetName == "RightHand" || targetName == "RightUpperArm") {
                part = FindChild("RightArm");
                if (part.Addr != 0) return part;
            }
            else if (targetName == "LeftLeg" || targetName == "LeftFoot" || targetName == "LeftUpperLeg") {
                part = FindChild("LeftLeg");
                if (part.Addr != 0) return part;
            }
            else if (targetName == "RightLeg" || targetName == "RightFoot" || targetName == "RightUpperLeg") {
                part = FindChild("RightLeg");
                if (part.Addr != 0) return part;
            }

            return RbxInstance(0);
        }

        uintptr_t GetPrimitivePtr() {
            return Coms->ReadMemory<uintptr_t>(Addr + Offsets::BasePart::Primitive);
        }


        Vec3 GetPos() {
            uintptr_t prim = GetPrimitivePtr();
            return Coms->ReadMemory<Vec3>(prim + Offsets::Primitive::Position);

        }

        Rotation GetRotation() {
            uintptr_t prim = GetPrimitivePtr();
            return Coms->ReadMemory<Rotation>(prim + Offsets::Primitive::Rotation);
        }

        // Camera specific Read/Write Rotation
        Mat3 GetCameraRotation() {
            return Coms->ReadMemory<Mat3>(Addr + Offsets::Camera::Rotation);
        }

        Vec3 GetCameraPosition() {
            return Coms->ReadMemory<Vec3>(Addr + Offsets::Camera::Position);
        }

        void SetCameraRotation(const Mat3& rot) {
            Coms->WriteMemory(Addr + Offsets::Camera::Rotation, rot);
        }

        RbxInstance GetModelRef() {
            return RbxInstance(Coms->ReadMemory<uintptr_t>(Addr + Offsets::Player::ModelInstance));
        }


        float CalcDistance(const Vec3& targetPos) {
            Vec3 currentPos = GetPos();

            
            float dx = currentPos.X - targetPos.X;
            float dy = currentPos.Y - targetPos.Y;
            float dz = currentPos.Z - targetPos.Z;
            
            return sqrtf(dx * dx + dy * dy + dz * dz);
        }

        bool IsInvincible() {
            
            uintptr_t childrenList = Coms->ReadMemory<uintptr_t>(Addr + Offsets::Instance::ChildrenStart);
            if (childrenList == 0) return false;

            uintptr_t childEnd = Coms->ReadMemory<uintptr_t>(childrenList + Offsets::Instance::ChildrenEnd);
            uintptr_t current = Coms->ReadMemory<uintptr_t>(childrenList);

            if (current == 0 || childEnd <= current) return false;

            size_t count = (childEnd - current) / 0x10;
            if (count > 50) count = 50; 

            for (size_t i = 0; i < count; ++i) {
                uintptr_t childAddr = Coms->ReadMemory<uintptr_t>(current + i * 0x10);
                if (childAddr != 0) {
                    RbxInstance childInst(childAddr);
                    if (childInst.GetClass() == "ForceField") {
                        return true;
                    }
                }
            }

            return false;
        }

        bool IsTransparent() {
            auto head = FindCharacterPart("Head");
            if (head.Addr == 0) return false;

            float transparency = Coms->ReadMemory<float>(head.Addr + Offsets::BasePart::Transparency);
            return (transparency > 0.01f); 
        }
    };


    class RenderEngine : public RbxInstance {
    public:
        RenderEngine(uintptr_t addr) : RbxInstance(addr) {}

        Mat4 GetViewMat() {
            return Coms->ReadMemory<Mat4>(Addr + Offsets::VisualEngine::ViewMatrix);
        }


        Vec2 WorldToViewport(const Vec3& worldPos) {
            Vec4 quat;
            
            Vec2 screenDims{ static_cast<float>(GetSystemMetrics(SM_CXSCREEN)), 
                            static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) };

            Mat4 viewMat = GetViewMat();
            
            quat.X = (worldPos.X * viewMat.data[0]) + (worldPos.Y * viewMat.data[1]) + (worldPos.Z * viewMat.data[2]) + viewMat.data[3];
            quat.Y = (worldPos.X * viewMat.data[4]) + (worldPos.Y * viewMat.data[5]) + (worldPos.Z * viewMat.data[6]) + viewMat.data[7];

            quat.Z = (worldPos.X * viewMat.data[8]) + (worldPos.Y * viewMat.data[9]) + (worldPos.Z * viewMat.data[10]) + viewMat.data[11];
            quat.W = (worldPos.X * viewMat.data[12]) + (worldPos.Y * viewMat.data[13]) + (worldPos.Z * viewMat.data[14]) + viewMat.data[15];
            
            Vec2 screenPos;
            
            if (quat.W < 0.1f) {

                return screenPos;
            }

            
            Vec3 ndc;
            ndc.X = quat.X / quat.W;
            ndc.Y = quat.Y / quat.W;
            ndc.Z = quat.Z / quat.W;
            
            screenPos.X = (screenDims.X / 2.0f * ndc.X) + (screenDims.X / 2.0f);
            screenPos.Y = -(screenDims.Y / 2.0f * ndc.Y) + (screenDims.Y / 2.0f);

            
            return screenPos;
        }
    };

    inline void ModifyWalkspeed(RbxInstance humanoid, float newSpeed) {
        Coms->WriteMemory(humanoid.Addr + Offsets::Humanoid::Walkspeed, newSpeed);
    }



    inline void ModifyJumpPower(RbxInstance humanoid, float newPower) {
        Coms->WriteMemory(humanoid.Addr + Offsets::Humanoid::JumpPower, newPower);
    }

    inline float GetNumberValue(RbxInstance valObj) {
        if (valObj.Addr == 0) return 0.0f;
        std::string className = valObj.GetClass();
        if (className == "NumberValue" || className == "DoubleValue") {
            return static_cast<float>(Coms->ReadMemory<double>(valObj.Addr + Offsets::Misc::Value));
        } else if (className == "IntValue") {
            return static_cast<float>(Coms->ReadMemory<int64_t>(valObj.Addr + Offsets::Misc::Value));
        }
        return 0.0f;
    }

}
