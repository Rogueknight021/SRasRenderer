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


## Display

### Shadow

<img src="Display/shadowmap.gif" width="1000">

### Tessellation

<img src="Display/tessellation.gif" width="1000">

### Skybox

<img src="Display/skybox.gif" width="1000">

### Anti-aliasing

<img src="Display/aliasing.png" width="1000"> <img src="Display/SSAA.png" width="1000">
<img src="Display/MSAA.png" width="1000"> <img src="Display/FXAA.png" width="1000">

> More details can be found in this video(https://www.bilibili.com/video/BV1yV4y1o74Z/?spm_id_from=333.999.0.0&vd_source=ec940e3d5e3d806957bf612f56681ee5)
## Third Party

* Qt5.15.2 as GUI
