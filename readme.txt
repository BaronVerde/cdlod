
General:

This is a naive take on a 2.5D (heightmap extrusion) terrain lod algorithm, basically for large and very large terrain. Pleae do not seperate or delete this file as it contains license information and attribution of work used in this project.

License:

Part of the algorithm, specifically the quadtree generation and selection algorithm in classes quadtree, node and lod_selection and terrain_lod shaders are modified versions of CDLOD, (c) 2009 Filip Strugar, published under the zlib license.

The rest of the source code, if not stated otherwise locally, is WTFPL licensed. So do as you please.

https://github.com/fstrugar/CDLOD
https://en.wikipedia.org/wiki/Zlib_License
https://en.wikipedia.org/wiki/WTFPL


Dependencies:

1.) stb_image
https://github.com/nothings/stb
I installed from Debian reps.
If you download the header, separate header and c source files as they are included in multiple locations.

2.) GLFW3
https://www.glfw.org/

3.) glad
https://glad.dav1d.de/
Separate header and c source files as they are included in multiple locations.
Specification OpenGL, core profile, Version 4.5

4.) Dear imgui
https://github.com/ocornut/imgui
