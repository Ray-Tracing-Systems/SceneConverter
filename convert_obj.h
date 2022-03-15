#ifndef SCENE_CONVERTER_CONVERT_OBJ_H
#define SCENE_CONVERTER_CONVERT_OBJ_H

#include <filesystem>

bool convert_obj_to_hydra(const std::filesystem::path& src_path, const std::filesystem::path& dest_path,
  bool export_single_mesh);

#endif //SCENE_CONVERTER_CONVERT_OBJ_H
