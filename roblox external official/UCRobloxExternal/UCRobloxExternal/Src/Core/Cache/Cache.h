#pragma once
#include "../../Game/SDK/SDK.h"
#include "../Globals/Globals.h"
#include <vector>
#include <string>
#include <algorithm>

namespace PlayerCache {

    struct CachedPlayer {
        uintptr_t playerAddr    = 0;
        uintptr_t characterAddr = 0;
        uintptr_t humanoidAddr  = 0;
        uintptr_t rootPartAddr  = 0;
        uintptr_t headAddr      = 0;
        uintptr_t torsoAddr     = 0;

        std::string name;
        std::string displayName;
        RBX::Vec3 position;
        RBX::Vec3 headPosition;
        float health    = 0.f;
        float maxHealth = 0.f;
        float distance  = 0.f;

        uintptr_t teamAddr  = 0;
        int       teamColor = -1;
        bool      isTeammate = false;

        bool isValid = false;
    };

    inline std::vector<CachedPlayer> players;
    inline RBX::Vec3 localPlayerPos;

    inline void UpdatePlayers() {
        auto playerList = Globals::players.GetChildList();

        uintptr_t localCharAddr = Coms->ReadMemory<uintptr_t>(Globals::localPlayer.Addr + Offsets::Player::ModelInstance);
        if (localCharAddr == 0) return;
        auto localChar = RBX::RbxInstance(localCharAddr);

        auto localRoot = localChar.FindChild("HumanoidRootPart");
        if (localRoot.Addr == 0) localRoot = localChar.FindChild("Root");
        if (localRoot.Addr == 0) return;
        
        localPlayerPos = localRoot.GetPos();

        // Read local team once before the loop instead of per-player
        uintptr_t localTeam = Coms->ReadMemory<uintptr_t>(Globals::localPlayer.Addr + Offsets::Player::Team);
        int localColor = -1;
        if (localTeam != 0) {
            localColor = Coms->ReadMemory<int>(localTeam + Offsets::Team::BrickColor);
        }

        for(auto& cached : players){
            cached.isValid = false;
        }

        for(auto& plr : playerList){
            if(plr.Addr == Globals::localPlayer.Addr) continue;

            CachedPlayer* existingPlayer = nullptr;
            for(auto& cached : players){
                if(cached.playerAddr == plr.Addr){
                    existingPlayer = &cached;
                    break;
                }
            }

            if(existingPlayer == nullptr){
                CachedPlayer newPlayer;
                newPlayer.playerAddr = plr.Addr;
                newPlayer.name = plr.GetName();
                newPlayer.displayName = Coms->ReadMemory<uintptr_t>(plr.Addr + Offsets::Player::DisplayName) ? Coms->ReadGameString(plr.Addr + Offsets::Player::DisplayName) : "";
                players.push_back(newPlayer);
                existingPlayer = &players.back();
            }

            auto character = RBX::RbxInstance(Coms->ReadMemory<uintptr_t>(plr.Addr + Offsets::Player::ModelInstance));
            if(character.Addr == 0) continue;
            
            existingPlayer->characterAddr = character.Addr;

            auto humanoid = character.FindChildByClass("Humanoid");
            existingPlayer->humanoidAddr = humanoid.Addr;

            auto rootPart = character.FindChild("HumanoidRootPart");
            if (rootPart.Addr == 0) rootPart = character.FindChild("Root");
            if (rootPart.Addr == 0) continue;

            auto head = character.FindChild("Head");

            auto torso = character.FindChild("UpperTorso");
            if (torso.Addr == 0) torso = character.FindChild("Torso");

            existingPlayer->rootPartAddr = rootPart.Addr;
            existingPlayer->position     = rootPart.GetPos();
            existingPlayer->headAddr     = head.Addr;
            existingPlayer->headPosition = head.Addr != 0 ? head.GetPos() : rootPart.GetPos();
            existingPlayer->torsoAddr    = torso.Addr;

            if (humanoid.Addr != 0) {
                existingPlayer->health    = Coms->ReadMemory<float>(humanoid.Addr + Offsets::Humanoid::Health);
                existingPlayer->maxHealth = Coms->ReadMemory<float>(humanoid.Addr + Offsets::Humanoid::MaxHealth);
            } else {
                auto healthVal = character.FindChild("Health");
                if (healthVal.Addr != 0) {
                    existingPlayer->health = RBX::GetNumberValue(healthVal);
                    existingPlayer->maxHealth = 100.0f;
                } else {
                    existingPlayer->health = 100.0f;
                    existingPlayer->maxHealth = 100.0f;
                }
            }
            
            existingPlayer->distance     = rootPart.CalcDistance(localPlayerPos);

            // Team check — pointer equality is the reliable primary check
            uintptr_t targetTeam = Coms->ReadMemory<uintptr_t>(plr.Addr + Offsets::Player::Team);
            existingPlayer->teamAddr = targetTeam;
            if (localTeam != 0 && targetTeam != 0) {
                if (localTeam == targetTeam) {
                    existingPlayer->isTeammate = true;
                } else {
                    int targetColor = Coms->ReadMemory<int>(targetTeam + Offsets::Team::BrickColor);
                    existingPlayer->teamColor  = targetColor;
                    existingPlayer->isTeammate = (localColor == targetColor);
                }
            } else {
                existingPlayer->teamColor  = -1;
                existingPlayer->isTeammate = false;
            }


            existingPlayer->isValid = true;
        }

        players.erase(
            std::remove_if(players.begin(), players.end(),
                [](const CachedPlayer& p) { return !p.isValid; }),
            players.end()
        );

    }
}
