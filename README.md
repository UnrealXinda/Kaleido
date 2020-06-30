# Kaleido
A UE4 plugin for manipulating instanced mesh transforms with compute shader.  
  
The plugin introduces "influencers", an aura-like concept, to carry multiple effects to affect the transforms (RTS) of overlapped "Kaleido"s (instanced meshes).  
  
By leveraging the power of C++ templates, the plugin enables fast binding of compute shaders via shader name at runtime, easy registering of compute shaders using simple macros and templates.  

## Sample Image
![kaleido](kaleido.gif)  

## Sample Video
[![](https://img.youtube.com/vi/3CpwZER5dYs/0.jpg)](https://www.youtube.com/watch?v=3CpwZER5dYs)
