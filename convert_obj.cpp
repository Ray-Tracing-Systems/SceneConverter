#include <string>
#include <iostream>
#include <filesystem>

#include "convert_common.h"
#include "convert_obj.h"

#include "HydraAPI.h"
#include "HydraXMLHelpers.h"


bool load_scene_obj(const std::filesystem::path& in_path, HRSceneInstRef scnRef);

bool convert_obj_to_hydra(const std::filesystem::path& src_path, const std::filesystem::path& dest_path)
{
  hrErrorCallerPlace(L"convert_obj_to_hydra");

  std::filesystem::create_directories(dest_path);

  hrSceneLibraryOpen(dest_path.wstring().c_str(), HR_WRITE_DISCARD);

  HRSceneInstRef scnRef = hrSceneCreate(L"exported_scene");

  load_scene_obj(src_path, scnRef);

  add_default_light(scnRef);
  auto camRef    = add_default_camera();
  auto defaultRender = add_default_render(0);

  hrSceneClose(scnRef);

  HRRenderRef renderRef;
  renderRef.id = -1;
  hrFlush(scnRef, renderRef, camRef);

  hrSceneLibraryClose();

  return true;
}


bool load_scene_obj(const std::filesystem::path& in_path, HRSceneInstRef scnRef)
{
  HRModelLoadInfo loadInfo{};
  loadInfo.useMaterial = true;
  loadInfo.useCentering = true;

  std::wstring pathW = in_path.wstring();
  auto objMesh = hrMeshCreateFromFile(pathW.c_str(), loadInfo);

  // geometry and instancing
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    LiteMath::float4x4 identityM;
    hrMeshInstance(scnRef, objMesh, identityM.L());
  }
 
  return true;
}