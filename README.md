<img src="https://github.com/Iscandel/Raytracer/blob/master/RaytracerNew2/main.jpg" width="50%"> <img src="https://github.com/Iscandel/Raytracer/blob/master/RaytracerNew2/main2.jpg" width="48%">

Physically based renderer, written in C++11. It comes with a strong inspiration from PBRT, Mitsuba and Nori, from which it follows the same design ideas.   
See: https://www.mitsuba-renderer.org/  
https://www.pbrt.org/  
https://wjakob.github.io/nori/  
# Features  
-Fully multithreaded (CPU) on available cores  
-BSDF: smooth and rough metal (resp. dielectric), Bump / normal mapping, car paint, lambert, multi layered, mirror, shadow catcher...  
-BSSRDF: fast dipole model  
-Textures: image (png, bmp, jpeg, OpenEXR support), Perlin noise based  
-Basic tone mapping  
-Lights: area, point, spot lights, environment map  
-Spectral / RGB handling  
-Samplers: random, stratified  
-rendering algorithms: direct, multiple importance sampling path tracing  
-Participating media: homogeneous, heterogenous, emissive medium  
-Volumes: Mitsuba grid, openVDB volume  
-Shapes: cube, sphere, triangle mesh, plane, implicit surfaces (currently, only extended to the mandelbulb fractal)  
