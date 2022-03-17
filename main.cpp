#include <filesystem>
#include <iostream>
#include "convert_common.h"
#include "convert_gltf.h"
#include "convert_obj.h"

#include <unordered_set>

void display_help()
{
  std::cout << "scene-converter converts input '.gltf'/'.obj' files into HydraXML scene.\n";
  std::cout << "Optionally, scene can be merged into a single '.vsgf2' mesh\n";
  std::cout << "Example usage:\n";
  std::cout << "\tscene-converter /path/to/scene.gltf /path/to/hydra/scenelib [OPTIONS]\n";
  std::cout << "[OPTIONS] can include:\n";
  std::cout << "\t --collapse - all meshes and their instances in the input scene will be merged into one, result will be saved as .vsgf2 file\n";
  std::cout << "\t --convert-materials - (only for .gltf scenes) pbr metallic-roughness materials will be approximately converted to native Hydra Renderer materials\n";
  std::cout << std::endl;
}

int main(int argc, char* argv[])
{
  if(argc < 3)
  {
    std::cout << "Not enough arguments. Usage:\n";
    display_help();
    return 1;
  }

  std::filesystem::path in_path(argv[1]);
  std::filesystem::path out_path(argv[2]);

  std::unordered_set<std::string> options;
  for (int i = 3; i < argc; ++i)
  {
    options.insert(argv[i]);
  }

  bool collapse_to_single_mesh = options.count("--collapse");
  bool convert_materials       = options.count("--convert-materials");
 
  if (!std::filesystem::exists(in_path))
  {
    std::cout << "Input scene " << in_path << "does not exist!\n";
    return 1;
  }


  bool result = true;
  switch (guessSceneTypeFromExt(in_path))
  {
  case SCENE_TYPE::SCENE_GLTF:
    result = convert_gltf_to_hydra(in_path, out_path, collapse_to_single_mesh, convert_materials);
    break;
  case SCENE_TYPE::SCENE_OBJ:
    result = convert_obj_to_hydra(in_path, out_path, collapse_to_single_mesh);
    break;
  case SCENE_TYPE::SCENE_UNKNOWN:
    result = false;
    std::cout << "Unknown scene type\n";
    break;
  default:
    result = false;
    break;
  }


  if (!result)
    return 1;
  
  std::cout << "Conversion completed. Output written to: " << out_path << std::endl;
  return 0;
}

