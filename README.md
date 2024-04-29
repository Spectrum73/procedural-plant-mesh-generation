# Procedural Generation of Level of Detail Meshes for Trees
Individual Project for Year 3 Computer Science.

## Requirements
The repository by itself includes all required libraries except from the MCUT library which is available at:
https://github.com/cutdigital/mcut \
Details on its installation can be found via:
https://cutdigital.github.io/mcut.site/building/

## Usage
The running the application opens a GUI which includes 4 panels "Plant Parameters, "Camera Settings", "Export Settings" and "Scene"
### Scene
The scene window displays the rendering time for the current frame in nanoseconds, below in the majority of the window is the rotating mesh of the current tree.
When the camera isn't set to Orbit this window can be controlled by holding the left mouse button over it and using the mouse to look around and the WASD keys to move, the shift key can also be used to increase the speed of the camera.
### Camera Settings
The shader of the tree can be set to three settings:
- Textured to display a simple bark texture.
- Basic Lit to display basic lighting
- Normals to colour the tree in accordance to its normal values.
A wireframe checkbox can also be ticked to enable wireframe view.
#### Orbit Settings
A checkbox can be toggled to adjust the camera mode.
The orbit distance, height and speed (including direction) can be adjusted via sliders within this section of the window.
### Export Settings
Here the file name of the exported meshes can be set, to export press "Save as Wavefront .OBJ"
The generated models are saved in the wavefront .OBJ format and are saved to `PlantGenerator/generations/`.
Three meshes will be saved for each tree with a suffix of `_0`, `_1` or `_2` denoting the LOD.
### Plant Settings
The "Regenerate" button will regenerate the current tree.
LOD Level can be changed to alter the current LOD mesh being viewed for the tree.
Light direction doesn't affect the lighting shader, but instead the direction of phototropism growth for the tree. \
\
Details on other parameters can be found within the associated report for this project.
