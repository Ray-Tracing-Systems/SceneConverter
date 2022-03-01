#ifndef SCENE_CONVERTER_CONVERT_GLTF_H
#define SCENE_CONVERTER_CONVERT_GLTF_H

constexpr int32_t HAPI_TEX_ID_OFFSET = 1; // HydraAPI reserves texture id = 0 for special texture
constexpr int32_t HAPI_MAT_ID_OFFSET = 1; // reserve mat_id = 0 for "no material" (gltf matid = -1)

bool convert_gltf_to_hydra(const char* src_path, const char* dest_path, bool convert_materials = false);

#endif //SCENE_CONVERTER_CONVERT_GLTF_H
