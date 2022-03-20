### Scene converter to HydraAPI format

Converts .gltf and .obj scenes to HydraXML format. Optionally, can also output single .vsgf2 mesh.

### Building

1. Clone alongside HydraAPI (i.e. SceneConverter dir and HydraAPI dir should be in the same parent directory).

2. Build using cmake (under Windows build with the same MSVC compiler as HydraAPI)

### Usage

```shell
scene-converter /path/to/scene.gltf /path/to/hydra/scenelib [OPTIONS]
```
Options can include:
* --collapse - all meshes and their instances in the input scene will be merged into one, result will be saved as .vsgf2 file
* --convert-materials - (only for .gltf scenes) pbr metallic-roughness materials will be approximately converted to native Hydra Renderer material format

### Dependencies:
1. [HydraAPI](https://github.com/Ray-Tracing-Systems/HydraAPI)
2. Single header libraries (LiteMath, tinygltf, stb_image) which are included in this repo.
    https://github.com/msu-graphics-group/LiteMath
    https://github.com/syoyo/tinygltf
    https://github.com/nothings/stb


