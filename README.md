# Exordia
Exordium (noun)\
pl. Exordia:\
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*A beginning or introduction especially to a discourse or **composition**.*\
-Merriam-Webster

Exordia is a modern graphics engine written in C++ using Vulkan. It is meant as a starting point for many projects, which is where the name comes from. I intend to expand the engine's capabilities through experiments ranging from graphics tech demos to physics simulation to small games. The end goal is a flexible, open source game engine with high fidelity graphics, modern C++ standards, and a customizable design.

## Features
3D rendering using Vulkan
- Reversed depth buffer with infinite far plane

Material system
- Reflection driven material properties
- Slang shaders, glsl coming soon
  - HLSL implicitly supported through Slang

Textures
- Automatic mipmap generation

Input system
- Simple GetKey syntax

Compute shaders (Work in progress)

Asset management system (Work in progress)
- Intrusive reference counting
- Automatic resource management
  - Resources are freed when no references remain

Dear ImGui integration

## Planned Features
2D Renderer
- Support for 9-slice texture mapping
- Support for quads and
- Integer depth buffer for faster sprite ordering
  - Standard
- Stencil buffer
- Compute driven tilemap system

UI System
- Built on 2D renderer

Event System
- Glorified function pointer manager

Multi-threading
- Main thread
  - Handles core loop and game logic
- Graphics thread
- Audio thread
- Physics thread
- Job system
  - Used to dispatch tasks to be run in parallel
  - Automates most of the threading logic

Input System 2.0
- Event driven
- Keep the easy to use methods like GetKey

Audio System

Editor
- Projects
- Asset management
  - Importing and preprocessing of assets
- Scene view
  - Rendering of gizmos
  - Scene hierarchy
  - Tool panels
- Inspector
  - Show and edit details of selected objects or assets
- Play and edit mode, with pausing and frame stepping
- Custom editors
- Building of a standalone runtime

Entity Component System hybrid architecture
- Object oriented for things with few instances, like players or game managers
- Data oriented for everything else, from enemies to projectiles

Particle Systems

2D Lighting

2D Physics Engine

3D Physics Engine

Global illumination

Ray tracing

3rd-party packages

Bindless render pipelines

## Build instructions
Build system: Cmake + Ninja + Clang

External dependencies can be found in CMakeLists.txt, ensure you have a recent version of Vulkan installed. I intend to improve the build system in the future, but in the meantime if you wish to use this as the starting point for your own projects you will have to figure out how to build it on your own. 
