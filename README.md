# Hydraulic Erosion
# Created by Daniel Tetreault

# Build instructions
The .exe file should run on any Windows 10/11 desktop with x86 architecture and
a modern GPU, with drivers that support Vulkan and compute shaders.
## Platform
This engine was built on Windows 11 with modern hardware.
Required static libraries (Not included in project):
- Vulkan 1.4.309.0
- glm
- glfw
Other libraries are included in the lib folder.
If you wish to build it on another platform or with vulkan directly imported,
that will take some doing on your part but it should work just fine. There is
some windows specific functionality in main.cpp, used to set the title bar and
task bar icon.

# How to use
Run the executable from the build folder.
Configure simulation parameters, then press "Run on GPU" to run the simulation.
When you press stop, it will stop.
The rest of the buttons are pretty self explanatory.
When saving to files, you have to manually type the file extension.

# Resources
Textures from www.polyhaven.com
