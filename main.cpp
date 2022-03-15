#include <filesystem>
#include <iostream>
#include "convert_common.h"
#include "convert_gltf.h"
#include "convert_obj.h"



int main(int argc, char* argv[])
{
  if(argc < 3)
  {
    std::cout << "Not enough arguments. Usage:\n";
    std::cout << "scene-converter /path/to/scene.gltf /path/to/hydra/scenelib";
    return 1;
  }

  std::filesystem::path in_path(argv[1]);
  std::filesystem::path out_path(argv[2]);
  
  if (!std::filesystem::exists(in_path))
  {
    std::cout << "Input scene " << in_path << "does not exist!\n";
    return 1;
  }

  bool result = true;
  switch (guessSceneTypeFromExt(in_path))
  {
  case SCENE_TYPE::SCENE_GLTF:
    result = convert_gltf_to_hydra(in_path, out_path, true);
    break;
  case SCENE_TYPE::SCENE_OBJ:
    result = convert_obj_to_hydra(in_path, out_path);
    break;
  case SCENE_TYPE::SCENE_UNKNOWN:
    result = false;
    std::cout << "Unknown scene type\n";
    break;
  default:
    result = false;
    break;
  }

  if (result)
  {
    std::cout << "Conversion completed. Output in: " << out_path << std::endl;
    return 0;
  }
  else
  {
    return 1;
  }
}

