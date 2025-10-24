## PlatformGame_GameDevelopment - Game Development Project Template

A complete 2D platformer game project template designed for Game Development coursework. This template provides a robust foundation for building a feature-rich platformer game across three progressive assignments, integrating SDL3, Box2D physics, Tiled map support, and advanced game systems.

## 🚀 Features

- **SDL3 Integration**: Latest SDL3 with automatic download and setup
- **Box2D Physics**: Integrated 2D physics engine for realistic movement
- **Tiled Support**: TMX map loading with pugixml
- **Image Loading**: SDL3_image with JPEG and PNG support
- **Cross-platform**: Windows (Visual Studio 2022) and Linux support
- **Automatic Dependency Management**: All libraries downloaded and configured automatically
- **Multi-architecture**: x64 and x86 platform support
- **Performance Tools**: Built-in FPS counter, frame capping, and profiling support

## 📁 Project Structure

```
PlatformerGame/
├── src/                      # Source code (.cpp files)
│   ├── PlatformGame.cpp     # Entry point
│   ├── Engine.cpp           # Core engine module
│   ├── Audio.cpp            # Audio system
│   ├── Input.cpp            # Input handling
│   ├── Log.cpp              # Logging utilities
│   ├── PerfTimer.cpp        # Performance timing
│   ├── Render.cpp           # Rendering system
│   ├── Scene.cpp            # Scene management
│   ├── Textures.cpp         # Texture management
│   ├── Timer.cpp            # Timer utilities
│   ├── Vector2D.cpp         # 2D vector math
│   └── Window.cpp           # Window management
│   
│   # Future structure for assignments:
│   ├── entities/            # Player, enemies, items (Entity system)
│   ├── physics/             # Box2D integration
│   ├── map/                 # TMX map loading
│   ├── pathfinding/         # A* pathfinding for enemies
│   └── gui/                 # GUI system (menus, HUD)
│
├── include/                 # Header files (.h files)
│   ├── Engine.h
│   ├── Module.h            # Base module class
│   ├── Audio.h
│   ├── Input.h
│   ├── Log.h
│   ├── PerfTimer.h
│   ├── Render.h
│   ├── Scene.h
│   ├── Textures.h
│   ├── Timer.h
│   ├── Vector2D.h
│   └── Window.h
│
├── assets/                  # Game assets
│   ├── textures/           # Sprites and tilesets (test.png)
│   └── audio/              # Audio files
│       └── music/          # Background music (level-iv-339695.wav)
│
├── build/                   # Build system
│   ├── premake5.exe        # Premake5 build tool (Windows)
│   ├── premake5.lua        # Build configuration script
│   ├── build_files/        # Generated Visual Studio projects
│   │   ├── PlatformerGame.vcxproj
│   │   ├── box2d.vcxproj
│   │   ├── pugixml.vcxproj
│   │   └── obj/            # Intermediate build files
│   └── external/           # External dependencies (auto-downloaded)
│       ├── SDL3/           # SDL3 library (3.2.24+)
│       ├── SDL3_image/     # Image loading library
│       ├── box2d/          # Box2D physics engine (3.1.1+)
│       ├── pugixml/        # XML parser for TMX/config files
│       ├── libjpeg-turbo/  # JPEG support
│       └── libpng/         # PNG support
│
├── bin/                     # Compiled executables and runtime files
│   ├── Debug/              # Debug build output
│   │   ├── PlatformerGame.exe
│   │   ├── PlatformerGame.pdb
│   │   ├── SDL3.dll
│   │   ├── SDL3_image.dll
│   │   ├── jpeg62.dll
│   │   ├── box2d.lib
│   │   └── pugixml.lib
│   └── Release/            # Release build output
│
├── .gitignore
├── build-VisualStudio2022.bat  # Quick build script
├── LICENSE
├── PlatformerGame.sln          # Visual Studio solution
└── README.md
```

## 🎮 Key Technologies & Features

### SDL3 Integration
Complete access to SDL3 capabilities:
- **Graphics**: 2D rendering with GPU backend support
- **Input**: Keyboard, mouse, gamepad support
- **Audio**: Music playback and sound effects
- **Window**: Multiple windows, fullscreen support
- **File I/O**: Cross-platform file operations
- **Threading**: Multithreading support

### Box2D Physics
Realistic 2D physics simulation:
- Rigid body dynamics
- Collision detection and response
- Joint and constraint systems
- Continuous collision detection (CCD)
- Efficient broad-phase collision

### Tiled Map Support
TMX map integration with pugixml:
- Orthographic map rendering
- Multiple layer support
- Dynamic collider generation from object layers
- Tileset management
- Custom properties support

### Performance Features
- **Frame Rate Control**: Stable 60 FPS cap
- **Delta Time**: Movement normalization
- **VSync Toggle**: Runtime vsync control
- **FPS Display**: Real-time performance monitoring
- **Tracy Integration**: Advanced profiling (Assignment 3)

## 📚 Learning Resources

### Official Documentation
- **SDL3 Documentation**: [SDL3 Wiki](https://wiki.libsdl.org/SDL3)
- **Box2D Manual**: [Box2D Documentation](https://box2d.org/documentation/)
- **Tiled Documentation**: [Tiled Map Editor](https://doc.mapeditor.org/)
- **Premake5**: [Premake5 Documentation](https://premake.github.io/docs/)

### Tutorials & Guides
- **Platformer Design**: [The Guide to Implementing 2D Platformers](http://higherorderfun.com/blog/2012/05/20/the-guide-to-implementing-2d-platformers/)
- **A* for Platformers**: [Adapting A* to Platformers](https://howtorts.github.io/2013/12/23/platformer-ai.html)
- **Box2D Tutorials**: [iforce2d's Box2D Tutorials](https://www.iforce2d.net/b2dtut/)
- **Game Architecture**: [Game Programming Patterns](https://gameprogrammingpatterns.com/)

### Asset Resources
- **Open Game Art**: [OpenGameArt.org](https://opengameart.org/)
- **Kenney Assets**: [kenney.nl](https://kenney.nl/assets)
- **itch.io**: [Game Assets on itch.io](https://itch.io/game-assets)
- **GameDev Market**: [gamedevmarket.net](https://www.gamedevmarket.net/)

### Community & Support
- **SDL Discord**: [SDL Community Discord](https://discord.com/invite/BwpFGBWsv8)
- **Box2D Forum**: [Box2D Discussions](https://github.com/erincatto/box2d/discussions)
- **Game Dev Reddit**: [r/gamedev](https://www.reddit.com/r/gamedev/)
- **TIGSource**: [TIGSource Forums](https://forums.tigsource.com/)