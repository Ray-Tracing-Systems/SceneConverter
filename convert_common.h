#ifndef SCENE_CONVERTER_CONVERT_COMMON_H
#define SCENE_CONVERTER_CONVERT_COMMON_H

#include "HydraAPI.h"
#include <filesystem>

constexpr int32_t HAPI_TEX_ID_OFFSET = 1; // HydraAPI reserves texture id = 0 for special texture
constexpr int32_t HAPI_MAT_ID_OFFSET = 1; // reserve mat_id = 0 for "no material" (gltf matid = -1)

enum class SCENE_TYPE
{
  SCENE_GLTF,
  SCENE_OBJ,
  SCENE_UNKNOWN
};

SCENE_TYPE guessSceneTypeFromExt(const std::filesystem::path& scenePath);

HRCameraRef add_default_camera();
HRRenderRef add_default_render(int32_t a_deviceId);
void add_default_light(HRSceneInstRef scnRef);



#endif //SCENE_CONVERTER_CONVERT_COMMON_H
