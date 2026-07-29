# Luminus Engine v1.0.0

<p align="center">
  <strong>A powerful, lightweight 2D/3D game engine built on Cocos2d-x v4</strong><br>
  <em>Custom scripting language • Visual editor • Cross-platform CI • Self-healing builds</em>
</p>

---

## What is Luminus Engine?

Luminus is a fork-derived game engine built on top of [Cocos2d-x v4](https://github.com/cocos2d/cocos2d-x) (MIT-licensed). We took the powerful, mature Cocos2d-x engine core and added:

- **LuminusScript** — a beginner-friendly scripting language (much easier than Lua)
- **Visual Editor** — built with Dear ImGui, includes Outliner, Inspector, Console
- **JSON Scene Format** — edit scenes with any text editor
- **Cross-platform CI** — builds for Windows, Linux, macOS via GitHub Actions
- **Self-healing CI** — automatic build-failure detection and patching

## Quick Start

### Download pre-built binaries

Go to [Actions tab](../../actions) → click latest successful build → download artifact for your OS.

### Build from source

```bash
git clone https://github.com/salim77088/engine.git
cd engine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
./build/bin/luminus
```

**Linux prerequisites:**
```bash
sudo apt-get install -y libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev \
  libxi-dev libgl1-mesa-dev libasound2-dev libgtk-3-dev libfontconfig1-dev \
  libglib2.0-dev pkg-config
```

## Usage

```bash
luminus                              # default scene + editor
luminus --scene path/to/scene.json   # custom scene
luminus --script path/to/script.ls   # load LuminusScript
luminus --no-editor                   # disable editor
luminus --width 1920 --height 1080   # custom resolution
```

### In-app keys
- `F1` — toggle visual editor
- `ESC` — quit

## LuminusScript — Easy Scripting

```
# Move player with arrow keys
when_start {
    say "GameStarted"
}

when_update {
    if key_held LEFT  { player.x -= 5 }
    if key_held RIGHT { player.x += 5 }
    if key_held UP    { player.y -= 5 }
    if key_held DOWN  { player.y += 5 }
    if key_hit SPACE  { player.y -= 20 }
    if key_held ESC   { exit }
}
```

### Supported statements
- `entity.property op value` — x, y, z, w, h, rotation, scale, opacity, visible, text, color
- `if key_held KEY { ... }` — held check (LEFT/RIGHT/UP/DOWN/SPACE/ENTER/ESC/A-Z/0-9)
- `if key_hit KEY { ... }` — pressed-once check
- `say "text"` — log to console
- `spawn NAME`, `destroy NAME`
- `load_scene "path"`, `set_background COLOR`, `set_fps N`, `exit`

## Scene Format

```json
{
  "name": "My Scene",
  "background": "DARKGRAY",
  "entities": [
    { "name": "player", "type": "sprite", "x": 100, "y": 200, "w": 32, "h": 32, "color": "SKYBLUE" },
    { "name": "label", "type": "text", "text": "Hello", "x": 50, "y": 50, "fontSize": 24, "color": "GOLD" }
  ]
}
```

## Architecture

```
src/
├── core/         Engine, SceneManager, Entity types
├── graphics/     GraphicsAdapter (Cocos2d-x wrapper)
├── scripting/    LuminusScript: Lexer, Parser, Interpreter
├── editor/       ImGui-based Editor (Outliner, Inspector, Console)
├── audio/        AudioSystem (Cocos2d-x AudioEngine wrapper)
└── assets/       AssetManager (FileUtils wrapper)
```

### Dependencies (auto-fetched via CMake FetchContent)
- [Cocos2d-x v4](https://github.com/cocos2d/cocos2d-x) — MIT (from our fork)
- [Dear ImGui 1.91.5](https://github.com/ocornut/imgui) — MIT
- [nlohmann/json 3.11.3](https://github.com/nlohmann/json) — MIT

## Self-Healing CI

Two workflows work together:
1. `build.yml` — builds for all 3 platforms, uploads artifacts
2. `autofix.yml` — triggers automatically when build fails, runs `tools/autofix.py` which:
   - Downloads failed job logs from GitHub API
   - Scans for known error patterns (missing includes, missing semicolons, wrong include paths)
   - Applies patches to source files
   - Commits and pushes the fix automatically
   - Triggers a new build automatically

## License

- **Luminus Engine wrapper code:** MIT
- **Cocos2d-x:** MIT
- **Dear ImGui:** MIT
- **nlohmann/json:** MIT

All components are 100% open source.

---

<p align="center">Built with Cocos2d-x + Dear ImGui • Powered by GitHub Actions</p>
