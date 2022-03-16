#include <string>
#include <iostream>
#include <filesystem>

#include "convert_common.h"
#include "convert_obj.h"

#include "HydraAPI.h"
#include "HydraXMLHelpers.h"


bool convert_obj_to_hydra(const std::filesystem::path& src_path, const std::filesystem::path& dest_path, bool export_single_mesh)
{
  hrErrorCallerPlace(L"convert_obj_to_hydra");

  auto sceneLib = dest_path;
  sceneLib.append("scenelib");
  std::filesystem::create_directories(sceneLib);

  hrSceneLibraryOpen(sceneLib.wstring().c_str(), HR_WRITE_DISCARD);

  HRModelLoadInfo loadInfo{};
  loadInfo.useMaterial = true;
  loadInfo.useCentering = true;

  std::wstring pathW = src_path.wstring();
  auto objMesh = hrMeshCreateFromFile(pathW.c_str(), loadInfo);

  if (export_single_mesh)
  {
    saveMesh(objMesh, dest_path, src_path.stem());
  }

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
