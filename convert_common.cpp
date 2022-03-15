#include "convert_common.h"
#include "LiteMath.h"
#include <filesystem>

SCENE_TYPE guessSceneTypeFromExt(const std::filesystem::path& scenePath)
{
  const std::filesystem::path obj_ext1(".obj");
  const std::filesystem::path obj_ext2(".OBJ");
  const std::filesystem::path gltf_ext1(".gltf");
  const std::filesystem::path gltf_ext2(".GLTF");

  if (scenePath.extension() == obj_ext1 || scenePath.extension() == obj_ext2)
    return SCENE_TYPE::SCENE_OBJ;
  else if (scenePath.extension() == gltf_ext1 || scenePath.extension() == gltf_ext2)
    return SCENE_TYPE::SCENE_GLTF;
  else
    return SCENE_TYPE::SCENE_UNKNOWN;
}

void add_default_light(HRSceneInstRef scnRef)
{
  HRLightRef sky = hrLightCreate(L"sky");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
  }
  hrLightClose(sky);

  LiteMath::float4x4 mat;
  hrLightInstance(scnRef, sky, mat.L());
}

HRCameraRef add_default_camera()
{
  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 0 10");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  return camRef;
}

HRRenderRef add_default_render(int32_t a_deviceId)
{
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  hrRenderEnableDevice(renderRef, a_deviceId, true);

  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    auto node = hrRenderParamNode(renderRef);

    node.append_child(L"width").text() = 1024;
    node.append_child(L"height").text() = 1024;

    node.append_child(L"method_primary").text() = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text() = L"pathtracing";
    node.append_child(L"method_caustic").text() = L"pathtracing";

    node.append_child(L"trace_depth").text() = 6;
    node.append_child(L"diff_trace_depth").text() = 3;
    node.append_child(L"maxRaysPerPixel").text() = 512;
    node.append_child(L"resources_path").text() = L"..";
    node.append_child(L"offline_pt").text() = 0;
  }
  hrRenderClose(renderRef);

  return renderRef;
}