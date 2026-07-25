#pragma once
#include "../../../Game/SDK/SDK.h"
#include "../../Vars/Vars.h"
#include "../../Globals/Globals.h"
#include <windows.h>
#include <vector>
#include <utility>

namespace Movement {

    inline void RunInfiniteJump() {
        if (!Vars::Local::infiniteJumpEnabled) return;

        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr == 0) return;

        auto hrp = character.FindChild("HumanoidRootPart");
        if (hrp.Addr == 0) hrp = character.FindChild("Root");
        if (hrp.Addr == 0) return;

        uintptr_t prim = hrp.GetPrimitivePtr();
        if (prim == 0) return;

        auto humanoid = character.FindChildByClass("Humanoid");
        if (humanoid.Addr == 0) return;

        static bool wasSpaceDown = false;
        bool isSpaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
        
        if (isSpaceDown && !wasSpaceDown) {
            RBX::Vec3 pos = Coms->ReadMemory<RBX::Vec3>(prim + Offsets::Primitive::Position);
            pos.Y += (Vars::Local::jumpPower * 0.05f);
            Coms->WriteMemory<RBX::Vec3>(prim + Offsets::Primitive::Position, pos);

            RBX::Vec3 currentVelocity = Coms->ReadMemory<RBX::Vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity);
            currentVelocity.Y = 0.0f;
            Coms->WriteMemory<RBX::Vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity, currentVelocity);
            
            uintptr_t stateObj = Coms->ReadMemory<uintptr_t>(humanoid.Addr + Offsets::Humanoid::HumanoidState);
            if (stateObj) {
                Coms->WriteMemory<BYTE>(stateObj + Offsets::Humanoid::HumanoidStateID, 5); 
            }
            Coms->WriteMemory<bool>(humanoid.Addr + Offsets::Humanoid::Jump, true);
        }
        wasSpaceDown = isSpaceDown;
    }

    inline void RunNoclip() {
        static bool wasKeyDown = false;

        if (Vars::Local::noclipKey <= 0 || Vars::Local::noclipKey >= 256) {
            if (Vars::Local::noclipEnabled) {
                Vars::Local::noclipEnabled = false;
            }
            return;
        }

        bool isKeyDown = (GetAsyncKeyState(Vars::Local::noclipKey) & 0x8000) != 0;
        if (isKeyDown && !wasKeyDown) {
            Vars::Local::noclipEnabled = !Vars::Local::noclipEnabled;
        }
        wasKeyDown = isKeyDown;

        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr == 0) return;

        auto parts = character.GetChildList();
        for (auto& part : parts) {
            std::string className = part.GetClass();
            if (className != "Part" && className != "MeshPart") continue;

            uintptr_t prim = part.GetPrimitivePtr();
            if (prim == 0) continue;

            DWORD flags = Coms->ReadMemory<DWORD>(prim + Offsets::Primitive::Flags);

            if (Vars::Local::noclipEnabled) {
                if (flags & Offsets::PrimitiveFlags::CanCollide) {
                    Coms->WriteMemory<DWORD>(prim + Offsets::Primitive::Flags, flags & ~Offsets::PrimitiveFlags::CanCollide);
                }
            } else {
                if (!(flags & Offsets::PrimitiveFlags::CanCollide)) {
                    Coms->WriteMemory<DWORD>(prim + Offsets::Primitive::Flags, flags | Offsets::PrimitiveFlags::CanCollide);
                }
            }
        }
    }

    inline void RunDesync() {
        static bool wasKeyDown = false;

        if (Vars::Local::desyncKey <= 0 || Vars::Local::desyncKey >= 256) {
            if (Vars::Local::desyncEnabled) {
                Vars::Local::desyncEnabled = false;
            }
            return;
        }

        bool isKeyDown = (GetAsyncKeyState(Vars::Local::desyncKey) & 0x8000) != 0;
        if (isKeyDown && !wasKeyDown) {
            Vars::Local::desyncEnabled = !Vars::Local::desyncEnabled;
        }
        wasKeyDown = isKeyDown;

        if (!Vars::Local::desyncEnabled) return;
        
        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr == 0) return;

        auto hrp = character.FindChild("HumanoidRootPart");
        if (hrp.Addr == 0) hrp = character.FindChild("Root");
        if (hrp.Addr == 0) return;
        
        uintptr_t prim = hrp.GetPrimitivePtr();
        if (prim == 0) return;
        
        if (Vars::Local::desyncType == 0) {
            if (Vars::Local::serverMode == 0) {
                RBX::Vec3 flingVelocity = {9e9f, 9e9f, 9e9f};
                Coms->WriteMemory<RBX::Vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity, flingVelocity);
            }
            else if (Vars::Local::serverMode == 1) {
                static int tickCount = 0;
                tickCount++;
                RBX::Vec3 realPos = Coms->ReadMemory<RBX::Vec3>(prim + Offsets::Primitive::Position);
                if (tickCount % 2 == 0) {
                    RBX::Vec3 fakePos = realPos;
                    fakePos.Y -= 1000.0f;
                    Coms->WriteMemory<RBX::Vec3>(prim + Offsets::Primitive::Position, fakePos);
                }
            }
        }
        else if (Vars::Local::desyncType == 1) {
            if (Vars::Local::clientMode == 0) {
                float nan = std::numeric_limits<float>::quiet_NaN();
                Coms->WriteMemory<RBX::Vec3>(prim + Offsets::Primitive::AssemblyAngularVelocity, {nan, nan, nan});
            }
            else if (Vars::Local::clientMode == 1) {
                float nan = std::numeric_limits<float>::quiet_NaN();
                Coms->WriteMemory<RBX::Vec3>(prim + Offsets::Primitive::AssemblyLinearVelocity, {nan, nan, nan});
            }
        }
    }
}
