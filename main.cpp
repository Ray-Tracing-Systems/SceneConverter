#include <string>
#include <iostream>
#include "convert_gltf.h"


int main(int argc, char* argv[])
{
  if(argc < 3)
  {
    std::cout << "Not enough arguments. Usage:\n";
    std::cout << "scene-converter /path/to/scene.gltf /path/to/hydra/scenelib";
    return 1;
  }

  std::string gltf_path(argv[1]);
  std::string out_path(argv[2]);

//  std::string gltf_path = "/home/vs/scenes/glTF-Sample-Models/2.0/Buggy/glTF/Buggy.gltf";
//  std::string gltf_path = "/home/vs/scenes/glTF-Sample-Models/2.0/WaterBottle/glTF/WaterBottle.gltf";
//  std::string hydra_out_path = "../hydra_out";

  auto res = convert_gltf_to_hydra(gltf_path.c_str(), out_path.c_str());

  return !res;
}

