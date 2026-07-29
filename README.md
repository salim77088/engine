# Luminus Engine v2.0

A powerful, lightweight, cross-platform game engine built on top of [Raylib 5.5](https://github.com/raysan5/raylib), [EnTT](https://github.com/skypjack/entt), and [raygui 4.0](https://github.com/raysan5/raygui). Featuring a custom scripting language, smart contract system, modern ECS architecture, and a full visual editor.

![License](https://img.shields.io/badge/License-MIT-blue)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-green)
![Language](https://img.shields.io/badge/Language-C%2B%2B17-orange)

## Features

### Core
- **Modern ECS** powered by EnTT (Entity-Component-System architecture)
- **Cross-platform** — builds natively for Windows, Linux, and macOS
- **Windowing** via Raylib 5.5 with MSAA, VSync, fullscreen support
- **High-precision timing** with FPS tracking and time scaling

### Graphics
- **2D and 3D rendering** with camera systems
- **Sprite batching** with draw order sorting
- **PBR materials** (albedo, metallic, roughness, emission)
- **Custom meshes** (Quad, Cube, Sphere, Plane)
- **Post-processing** effects (bloom, vignette, chromatic aberration)
- **Custom shaders** with hot-reload support

### Scripting — LuminusScript v2
A complete custom programming language with:
- Lexer (tokenizer) — 40+ token types
- Recursive-descent parser → AST
- Tree-walking interpreter
- Variables (`var`), constants (`const`), functions (`fun`)
- Control flow: `if`/`else`, `while`, `for` (including for-each)
- Event system: `on "EventName" -> { ... }` and `emit "EventName"`
- Entity access via `self` keyword
- List literals and operations
- Native bindings to engine APIs (input, time, spawning)

### Smart Game Contracts
Verifiable game logic system inspired by blockchain smart contracts:
```luminus
contract "PlayerSafety" {
    verify playerHealth > 0
    verify playerHealth <= MAX_HEALTH
}
```
- Rule-based contracts with conditions and actions
- Violation tracking and statistics
- Snapshot/verification for state validation
- Built-in contracts: EntityHealth, Performance

### Editor
Full visual editor with:
- **Hierarchy panel** — entity tree view
- **Inspector panel** — component editing
- **Console panel** — log viewer + live script execution
- **Asset browser** — file system navigation
- **Toolbar** — Play/Stop, Save/Load scene
- **Status bar** — FPS, entity count, mode

### Physics
- AABB collision detection
- Rigid body dynamics with gravity and drag
- Ray casting (2D)
- Spatial queries (AABB overlap)

### Audio
- Sound effects and music streaming
- Volume control per channel
- Hot-reload of audio assets

## Build

### Prerequisites
- CMake 3.16+
- C++17 compiler (MSVC 2019+, GCC 9+, Clang 10+)

### Linux
```bash
sudo apt install libasound2-dev libx11-dev libxrandr-dev \
    libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

### Windows
```cmd
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### macOS
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
```

## Usage

```bash
./luminus                              # Start with editor + default scene
./luminus --scene examples/demo.scene.json
./luminus --fullscreen --no-editor     # Game-only mode
./luminus --width 1920 --height 1080
```

## LuminusScript Example

```luminus
// Variable declarations
var playerHealth = 100
const MAX_HEALTH = 100
var frameCount = 0

// Function declaration
fun calculateDamage(base, multiplier) {
    return base * multiplier
}

// Entity lifecycle hooks
fun onStart(self) {
    print("Entity " + self + " started!")
}

fun onUpdate(self, dt) {
    frameCount = frameCount + 1
    
    // Movement with arrow keys
    if (keyDown(263)) {  // KEY_LEFT
        self.x = self.x - 200 * dt
    }
}

// Event system
on "GameStart" -> {
    print("Game started!")
}

// Smart contract — verifiable game rule
contract "PlayerSafety" {
    verify playerHealth > 0
    verify playerHealth <= MAX_HEALTH
}

// List iteration
fun sum(items) {
    var total = 0
    for (var x in items) {
        total = total + x
    }
    return total
}
```

## Architecture

```
src/
├── core/           # Engine, Window, Input, Time, SceneManager (ECS)
├── graphics/       # Renderer, Camera, Material, Mesh, Shader, PostProcessor
├── scripting/      # LuminusScript: Lexer, Parser, Interpreter
├── editor/         # Editor + panels (Hierarchy, Inspector, Console, AssetBrowser)
├── audio/          # AudioSystem
├── assets/         # AssetManager with hot-reload
├── physics/        # PhysicsWorld (AABB, raycast, queries)
├── contracts/      # SmartContract + ContractVM
└── utils/          # Logger, FileIO
```

## License

MIT License — fork it, modify it, ship it.

## Acknowledgments

Built on the shoulders of giants:
- [Raylib](https://github.com/raysan5/raylib) — Zlib license
- [EnTT](https://github.com/skypjack/entt) — MIT license
- [raygui](https://github.com/raysan5/raygui) — Zlib license
- [nlohmann/json](https://github.com/nlohmann/json) — MIT license
- [stb](https://github.com/nothings/stb) — Public Domain
