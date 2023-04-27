# SRasRenderer
A SoftRas Renderer based on CPU without third party besides Qt as Ui
<img src="Display/display2.png">
<img src="Display/display1.png">
<img src="Display/display3.png">
<img src="Display/display4.png">
## Features
### Load Module
* Model(only .obj)
* Texture including diffuse, normal and specular
* Cubemapped Skybox
### Render Pipeline
#### Vertex Shader
* MVP Matrix
* TBN Matrix
* Transformation of Normal(Model Space->View Space)
#### Fragment Shader
* Blinn-Phong reflection model
#### Tessellation Shader
* Phong tessellation
#### Culling
* View Frustum Culling
* Viewport Culling(Sutherland-Hodgman)
* Back-face Culling
#### Rasteration
* Early-z
* Perspective correct interpolation
* Texture mapping
#### Anti-aliasing
* Oversampling including SSAA and MSAA
* Post Processing including FXAA
#### Shadow
* Screen space shadowmap
#### Others
* Bresenham's line algorithm
* Loop Subivison
* Multithread acceleration
* Many interactive features
## Third Party
* QT 5.15.2 as GUI
