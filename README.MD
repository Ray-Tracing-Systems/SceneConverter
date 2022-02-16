### Scene converter to HydraAPI format

Currently, converts only gltf scenes without extensions.

### Building

Tested only under Linux.

1. Clone alongside HydraAPI (i.e. SceneConverter dir and HydraAPI dir should be in the same parent directory).

2. Build using cmake.

### Usage

```shell
scene-converter /path/to/scene.gltf /path/to/hydra/scenelib
```

### Dependencies:
1. [HydraAPI](https://github.com/Ray-Tracing-Systems/HydraAPI)
2. Single header libraries (LiteMath, tinygltf, stb_image) which are included in this repo.
    https://github.com/msu-graphics-group/LiteMath
    https://github.com/syoyo/tinygltf
    https://github.com/nothings/stb


