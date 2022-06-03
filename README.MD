### Scene converter to HydraAPI format

Converts .gltf and .obj scenes to HydraXML format. Optionally, can also output single .vsgf2 mesh.

### Building

1. Clone and build [HydraAPI](https://github.com/Ray-Tracing-Systems/HydraAPI).
2. Clone SceneConverter.
3. Configure using CMake - set paths to HydraAPI sources and dependencies, USE_GL to the same value as your HydraAPI build.
4. Build with GCC on Linux or MSVC on Windows (use the same MSVC version as HydraAPI)

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


