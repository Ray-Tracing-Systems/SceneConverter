#ifndef SCENE_CONVERTER_CONVERT_GLTF_H
#define SCENE_CONVERTER_CONVERT_GLTF_H

#include <filesystem>

bool convert_gltf_to_hydra(const std::filesystem::path &src_path, const std::filesystem::path& dest_path, bool convert_materials = false);

#endif //SCENE_CONVERTER_CONVERT_GLTF_H
