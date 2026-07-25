#include "Memory/Communication.h"
#include "Game/Offsets/Offsets.h"
#include "Game/SDK/SDK.h"
#include "Render/Render.h"
#include "Core/Globals/Globals.h"
#include "Core/Cache/Cache.h"
#include "Core/Features/Visuals/Visuals.h"
#include "Core/Features/Aimbot/Aimbot.h"
#include "Core/Features/Flight/Flight.h"
#include "Core/Features/Explorer/Explorer.h"
#include "Core/Features/World/World.h"
#include "Core/Features/Movement/Movement.h"
#include "Core/Features/Rage/Rage.h"
#include "Core/Config/Config.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <atomic>
#include <filesystem>
#include <random>

bool IsGameRunning(const wchar_t* windowTitle)
{
    HWND hwnd = FindWindowW(NULL, windowTitle);
    return hwnd != NULL;
}

std::atomic<bool> running(true);


void LocalPlayerThread() {
    static bool prevSpeedEnabled  = false;
    static bool prevJumpEnabled   = false;
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> jitter(-50, 50);

    while (running) {
        auto character = Globals::localPlayer.GetModelRef();
        if (character.Addr == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            continue;
        }

        auto humanoid = character.FindChildByClass("Humanoid");
        if (humanoid.Addr != 0) {
            if (Vars::Local::speedEnabled) {
                RBX::ModifyWalkspeed(humanoid, Vars::Local::walkSpeed);
                prevSpeedEnabled = true;
            } else if (prevSpeedEnabled) {
                RBX::ModifyWalkspeed(humanoid, 16.0f);
                prevSpeedEnabled = false;
            }

            if (Vars::Local::jumpEnabled) {
                RBX::ModifyJumpPower(humanoid, Vars::Local::jumpPower);
                prevJumpEnabled = true;
            } else if (prevJumpEnabled) {
                RBX::ModifyJumpPower(humanoid, 50.0f);
                prevJumpEnabled = false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250 + jitter(rng)));
    }
}

void LogWatcherThread() {
    char* localAppData;
    size_t len;
    _dupenv_s(&localAppData, &len, "LOCALAPPDATA");
    if (!localAppData) return;
    
    std::string logDir = std::string(localAppData) + "\\Roblox\\logs";
    free(localAppData);
    
    std::string currentLogFile = "";
    
    while (running) {
        if (!Vars::Misc::autoRescan) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        std::string latestLog = "";
        std::filesystem::file_time_type latestTime;
        bool first = true;

        if (std::filesystem::exists(logDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(logDir)) {
                if (entry.path().extension() == ".log") {
                    if (first || entry.last_write_time() > latestTime) {
                        latestLog = entry.path().string();
                        latestTime = entry.last_write_time();
                        first = false;
                    }
                }
            }
        }

        if (latestLog.empty()) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        if (latestLog != currentLogFile) {
            currentLogFile = latestLog;
        }

        std::ifstream logFile(currentLogFile, std::ios::in | std::ios::binary);
        if (!logFile.is_open()) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        logFile.seekg(0, std::ios::end);
        std::string line;

        while (running && Vars::Misc::autoRescan && currentLogFile == latestLog) {
            if (std::getline(logFile, line)) {
                if (line.find("! Joining game") != std::string::npos || line.find("Teleporting to") != std::string::npos) {
                    Vars::Misc::rescan = true;
                    NotificationService::Push("Server change detected, Rescanning...", 3.5f);
                }
            } else {
                logFile.clear();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                // Re-check for new log file every once in a while
                static int checkCounter = 0;
                if (++checkCounter > 10) {
                    checkCounter = 0;
                    break;
                }
            }
        }
    }
}

int main() {
    std::cout << "--- NxghtWatch by Nxght_Cry0 ---\n\n";
    std::cout << "[*] Searching for Roblox...\n";
    
    while (!Coms->Connect(L"RobloxPlayerBeta.exe")) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    system("cls");
    
    auto baseAddr = Coms->GetBase();
    std::cout << "[+] Process ID: " << Coms->GetPID() << "\n";
    std::cout << "[+] Base Address: 0x" << std::hex << baseAddr << std::dec << "\n";
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    system("cls");

    // ── Version check FIRST — before any memory reads ─────────────────────
    if (!Vars::Misc::fastLaunch) 
    {
        wchar_t processPath[MAX_PATH];
        DWORD size = MAX_PATH;
        std::string detectedVersion = "Unknown";
        if (QueryFullProcessImageNameW(Coms->GetHandle(), 0, processPath, &size)) {
            std::filesystem::path path(processPath);
            detectedVersion = path.parent_path().filename().string();
        }

        if (detectedVersion != Offsets::ClientVersion) {
            std::cout << "[!] Version Mismatch Detected!\n";
            std::cout << "[*] Offsets are for [" << Offsets::ClientVersion << "] but you are on [" << detectedVersion << "]\n";
            std::cout << "[!] Running with wrong offsets will corrupt game memory and break movement!\n";
            std::cout << "[?] Would you still like to launch? (Y/N): ";
            char choice;
            std::cin >> choice;
            if (choice != 'y' && choice != 'Y') {
                return 0;
            }
        }
        else {
            std::cout << "[+] Roblox version matches offsets, launching...\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        system("cls");
    }
    
    // ── Read DataModel pointers ───────────────────────────────────────────
    auto fakeDataModelAddr = baseAddr + Offsets::FakeDataModel::Pointer;
    auto fakeDataModel = Coms->ReadMemory<uintptr_t>(fakeDataModelAddr);
    auto dataModelAddr = fakeDataModel + Offsets::FakeDataModel::RealDataModel;
    auto dataModelPtr = Coms->ReadMemory<uintptr_t>(dataModelAddr);

    
    auto visualEngineAddr = baseAddr + Offsets::VisualEngine::Pointer;
    auto visualEngine = Coms->ReadMemory<uintptr_t>(visualEngineAddr);

    // ── Validate DataModel is sane ────────────────────────────────────────
    if (dataModelPtr == 0 || fakeDataModel == 0) {
        std::cout << "[!] Failed to read DataModel! Offsets may be wrong for this Roblox version.\n";
        std::cout << "[*] FakeDataModel: 0x" << std::hex << fakeDataModel << "\n";
        std::cout << "[*] DataModel: 0x" << std::hex << dataModelPtr << "\n";
        std::cout << "[!] Press any key to exit...\n";
        std::cin.get();
        return 1;
    }

    Globals::dataModel = RBX::RbxInstance(dataModelPtr);
    Globals::renderEngine = RBX::RenderEngine(visualEngine);
    Globals::workspace = Globals::dataModel.FindChildByClass("Workspace");
    Globals::players = Globals::dataModel.FindChildByClass("Players");
    Globals::camera = Globals::workspace.FindChildByClass("Camera");

    // ── Validate critical pointers ────────────────────────────────────────
    if (Globals::workspace.Addr == 0 || Globals::players.Addr == 0 || Globals::camera.Addr == 0) {
        std::cout << "[!] Could not find Workspace/Players/Camera! Offsets may be wrong.\n";
        std::cout << "[*] Workspace: 0x" << std::hex << Globals::workspace.Addr << "\n";
        std::cout << "[*] Players: 0x" << std::hex << Globals::players.Addr << "\n";
        std::cout << "[*] Camera: 0x" << std::hex << Globals::camera.Addr << "\n";
        std::cout << "[!] Press any key to exit...\n";
        std::cin.get();
        return 1;
    }

    std::cout << "[*] Children of DataModel:\n";
    for (auto& child : Globals::dataModel.GetChildList()) {
        std::cout << "  - Name: " << child.GetName() << " | Class: " << child.GetClass() << " | Addr: 0x" << std::hex << child.Addr << std::dec << "\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));

    auto localPlayerAddr = Coms->ReadMemory<uintptr_t>(Globals::players.Addr + Offsets::Player::LocalPlayer);
    Globals::localPlayer = RBX::RbxInstance(localPlayerAddr);

    system("cls");

    Config::Load("default");

    std::cout << "[+] DataModel: 0x" << std::hex << dataModelPtr << std::dec << "\n";
    std::cout << "[+] VisualEngine: 0x" << std::hex << visualEngine << std::dec << "\n";
    std::cout << "[+] Workspace: 0x" << std::hex << Globals::workspace.Addr << std::dec << "\n";
    std::cout << "[+] Players: 0x" << std::hex << Globals::players.Addr << std::dec << "\n";
    std::cout << "[+] Camera: 0x" << std::hex << Globals::camera.Addr << std::dec << "\n";
    std::cout << "[+] LocalPlayer: 0x" << std::hex << localPlayerAddr << std::dec << "\n\n";



    OverlayWindow overlay;
    if (!overlay.Initialize()) {
        std::cout << "[!] Failed to initialize overlay\n";
        return -1;
    }

    std::cout << "[+] Overlay initialized\n";
    std::cout << "[*] Press INSERT to toggle menu\n\n";

    std::thread localThread(LocalPlayerThread);
    std::thread logWatcher(LogWatcherThread);

    NotificationService::Push("Injected successfully", 3.0f);

    while (Coms->IsConnected()) 
    {
        if (!IsGameRunning(L"Roblox")) 
        {
            break;
        }

        if (GetAsyncKeyState(VK_INSERT) & 1) {
            Vars::menuOpen = !Vars::menuOpen;
        }

        if (Vars::Misc::rescan) {
            auto fakeDataModelAddr = baseAddr + Offsets::FakeDataModel::Pointer;
            auto fakeDataModel = Coms->ReadMemory<uintptr_t>(fakeDataModelAddr);
            auto dataModelAddr = fakeDataModel + Offsets::FakeDataModel::RealDataModel;
            auto dataModelPtr = Coms->ReadMemory<uintptr_t>(dataModelAddr);

            auto visualEngineAddr = baseAddr + Offsets::VisualEngine::Pointer;
            auto visualEngine = Coms->ReadMemory<uintptr_t>(visualEngineAddr);

            Globals::dataModel = RBX::RbxInstance(dataModelPtr);
            Globals::renderEngine = RBX::RenderEngine(visualEngine);
            Globals::workspace = Globals::dataModel.FindChildByClass("Workspace");
            Globals::players = Globals::dataModel.FindChildByClass("Players");
            Globals::camera = Globals::workspace.FindChildByClass("Camera");
            
            auto locPlr = Coms->ReadMemory<uintptr_t>(Globals::players.Addr + Offsets::Player::LocalPlayer);
            Globals::localPlayer = RBX::RbxInstance(locPlr);
            
            Vars::Misc::rescan = false;
            NotificationService::Push("Pointers rescanned", 2.0f);
        }

        static int frameCounter = 0;
        if(frameCounter % 3 == 0){
            PlayerCache::UpdatePlayers();
        }
        frameCounter++;

        overlay.BeginFrame();
        
        overlay.RenderMenu();
                   
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        static auto lastTime = std::chrono::high_resolution_clock::now();
        static int frameCount = 0;
        static int fps = 0;

        frameCount++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

        if (elapsed >= 1000) {
            fps = frameCount;
            frameCount = 0;
            lastTime = currentTime;
        }

        std::string watermark = "NxghtWatch | by Nxght_Cry0 | FPS: " + std::to_string(fps);
        ImVec2 textSize = ImGui::CalcTextSize(watermark.c_str());
        float screenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        float screenH = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
        ImVec2 watermarkPos = ImVec2(screenW - textSize.x - 10, 10);

        drawList->AddText(ImVec2(watermarkPos.x - 1, watermarkPos.y), IM_COL32(0, 0, 0, 255), watermark.c_str());
        drawList->AddText(ImVec2(watermarkPos.x + 1, watermarkPos.y), IM_COL32(0, 0, 0, 255), watermark.c_str());
        drawList->AddText(ImVec2(watermarkPos.x, watermarkPos.y - 1), IM_COL32(0, 0, 0, 255), watermark.c_str());
        drawList->AddText(ImVec2(watermarkPos.x, watermarkPos.y + 1), IM_COL32(0, 0, 0, 255), watermark.c_str());
        drawList->AddText(watermarkPos, IM_COL32(255, 255, 255, 255), watermark.c_str());

        if (Vars::Aimbot::enabled && Vars::Aimbot::showFOV) {
            POINT p;
            GetCursorPos(&p);
            ImVec2 center = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
            drawList->AddCircle(center, Vars::Aimbot::fovRadius, IM_COL32(0, 0, 0, 255), 64, 2.0f);
            drawList->AddCircle(center, Vars::Aimbot::fovRadius, IM_COL32(255, 255, 255, 255), 64, 1.0f);
        }

        if (Vars::Aimbot::silentAim && Vars::Aimbot::showSilentFOV) {
            POINT p;
            GetCursorPos(&p);
            ImVec2 center = ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
            drawList->AddCircle(center, Vars::Aimbot::silentFOV, IM_COL32(0, 0, 0, 255), 64, 2.0f);
            drawList->AddCircle(center, Vars::Aimbot::silentFOV, IM_COL32(255, 100, 100, 255), 64, 1.0f);
        }

        if (Vars::ESP::crosshair) {
            POINT p;
            GetCursorPos(&p);
            float cx = static_cast<float>(p.x);
            float cy = static_cast<float>(p.y);
            float size = Vars::ESP::crosshairSize;
            float gap = Vars::ESP::crosshairGap;
            float thick = Vars::ESP::crosshairThickness;
            ImU32 cCol = IM_COL32(
                (int)(Vars::ESP::crosshairColor[0] * 255),
                (int)(Vars::ESP::crosshairColor[1] * 255),
                (int)(Vars::ESP::crosshairColor[2] * 255),
                (int)(Vars::ESP::crosshairColor[3] * 255));
            ImU32 outlineCol = IM_COL32(0, 0, 0, 255);

            auto drawLine = [&](ImVec2 p1, ImVec2 p2) {
                drawList->AddLine(p1, p2, outlineCol, thick + 2.0f);
                drawList->AddLine(p1, p2, cCol, thick);
            };

            drawLine(ImVec2(cx - gap - size, cy), ImVec2(cx - gap, cy));
            drawLine(ImVec2(cx + gap, cy), ImVec2(cx + gap + size, cy));
            drawLine(ImVec2(cx, cy - gap - size), ImVec2(cx, cy - gap));
            drawLine(ImVec2(cx, cy + gap), ImVec2(cx, cy + gap + size));

            if (Vars::ESP::crosshairDot) {
                drawList->AddCircleFilled(ImVec2(cx, cy), thick + 1.0f, outlineCol);
                drawList->AddCircleFilled(ImVec2(cx, cy), thick, cCol);
            }
        }

        auto viewMatrix = Globals::renderEngine.GetViewMat();
        
        Flight::RunFlight();
        Movement::RunInfiniteJump();
        Movement::RunNoclip();
        Movement::RunDesync();
        Aimbot::RunAimbot(viewMatrix);
        Aimbot::RunTriggerbot(viewMatrix);
        Rage::RunRage();
        Visuals::RenderESP(drawList, viewMatrix);
        Explorer::RenderExplorer();
        World::RunWorld();
        
        NotificationService::Render(drawList);
        
        overlay.EndFrame();
    }
    
    running = false;
    localThread.join();
    logWatcher.join();

    overlay.Cleanup();
    
    return 0;
}
