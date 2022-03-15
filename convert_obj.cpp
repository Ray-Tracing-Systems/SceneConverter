#include <string>
#include <iostream>
#include <filesystem>

#include "convert_common.h"
#include "convert_obj.h"

#include "HydraAPI.h"
#include "HydraXMLHelpers.h"


bool convert_obj_to_hydra(const std::filesystem::path& src_path, const std::filesystem::path& dest_path, bool)
{
  hrErrorCallerPlace(L"convert_obj_to_hydra");

  std::filesystem::create_directories(dest_path);

  hrSceneLibraryOpen(dest_path.wstring().c_str(), HR_WRITE_DISCARD);

  HRModelLoadInfo loadInfo{};
  loadInfo.useMaterial = true;
  loadInfo.useCentering = true;

  std::wstring pathW = src_path.wstring();
  auto objMesh = hrMeshCreateFromFile(pathW.c_str(), loadInfo);

  HRSceneInstRef scnRef = hrSceneCreate(L"exported_scene");

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    LiteMath::float4x4 identityM;
    hrMeshInstance(scnRef, objMesh, identityM.L());
  }

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
