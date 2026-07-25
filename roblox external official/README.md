# Roblox External

This external is made for educational purposes only.
It is not intended for any malicious use.
Use only in games where exploiting is allowed, or use it to test your own anticheats.

## Download

Go to the [Releases](../../releases) page and download the latest `UCRobloxExternal.exe`.

## Building from source

**Requirements:** Visual Studio 2022 with the "Desktop development with C++" workload.

```
msbuild UCRobloxExternal/UCRobloxExternal/UCRobloxExternal.vcxproj /p:Configuration=Release /p:Platform=x64
```

The compiled `.exe` will be at `UCRobloxExternal/x64/Release/UCRobloxExternal.exe`.

## Features

- Aimbot (standard, silent aim, triggerbot)
- ESP (2D/Corner/3D boxes, skeleton, tracers, health bars)
- Flight, noclip, infinite jump
- World modifications (skybox, fog, lighting, FOV)
- Anti-aim
- Instance explorer
- Config save/load (Base64 JSON)
- Stream-proof mode

## made by nxght_Cry0 on discord yeah