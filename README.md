# Slayer Engine

## Project goals

* Small, lightweight game engine
* Primary focus on the runtime, not on a user-friendly editor

## Features

### Runtime
- [x] ECS: (Custom)
- [x] 3D Renderer: (OpenGL or DirectX 11)
  - [x] Forward rendering
  - [x] PBR rendering
  - [ ] FXAA Anti-aliasing
  - [x] MSAA Anti-aliasing
  - [x] Cascaded shadow mapping
  - [x] GPU-based skeletal animation 
  <!-- - [ ] Screen-space ambient occlusion -->
  <!-- - [ ] Screen-space reflections -->
  <!-- - [ ] Screen-space refraction -->
  <!-- - [ ] Depth of field -->
  <!-- - [ ] Bloom -->
- [ ] Asset Pipeline:
  - [x] Asset compilation
  - [ ] Asset compression
  - [x] Asset packaging
  - [ ] Asset streaming
- [x] Animation System
- [ ] Physcis: (Jolt)
- [ ] Audio: (OpenAL, SteamAudio)
- 

### Editor
- [x] Scene Editor(ImGUI)

## Getting Started

*Prerequisites*: Cmake, C++ compiler(MSVC or Clang)

First, clone the repository. In most cases, it's recommended to clone the dev branch. Also, ensure that you clone the submodules of the repository using the --recursive argument when cloning.
```bat
git clone https://github.com/Dualsub/slayer.git --branch dev --recursive
```

Then, run the setup script to build the executable and compile the assets.
```bat
cd Slayer
Scripts/setup.bat
```

Now you should have a executable in the build folder and a `.slp`-file in the assets folder. You can run the testbed-application by running the executable.