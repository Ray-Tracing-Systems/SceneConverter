#include "convert_common.h"
#include "LiteMath.h"
#include "HydraAPI.h"
#include "cmesh.h"

#include <filesystem>
#include <iostream>
#include <cassert>
#include <cstring>

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

bool copyExportedMesh(const std::filesystem::path& sceneLib, const std::filesystem::path& outDir, const std::filesystem::path& meshName)
{
  std::filesystem::path meshPath = sceneLib;
  meshPath.append("data");
  meshPath.append("chunk_00001.vsgf");

  if (!std::filesystem::exists(meshPath))
  {
    std::cout << "Unknown ERROR! Can't find converted mesh at " << meshPath << std::endl;
    return false;
  }

  std::filesystem::path resPath = outDir;
  resPath.append(meshName.native());

  std::filesystem::copy_file(meshPath, resPath);
  std::filesystem::remove_all(sceneLib);

  return true;
}

void saveMesh(HRMeshRef meshRef, const std::filesystem::path& outDir, const std::filesystem::path& meshName)
{
  std::filesystem::path actualMeshPath = outDir;
  std::filesystem::path ext(".vsgf2");
  if (meshName.extension() != ext)
  {
    actualMeshPath.append(meshName.filename().native());
    actualMeshPath.replace_extension(ext.native());
  }
  else
  {
    actualMeshPath.replace_extension(meshName.native());
  }
  hrMeshSaveVSGF(meshRef, actualMeshPath.wstring().c_str());
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

cmesh::SimpleMesh transformSimpleMesh(const cmesh::SimpleMesh& mesh, const LiteMath::float4x4& matrix)
{
  cmesh::SimpleMesh transformed = mesh;

  auto normalMatrix = LiteMath::transpose(LiteMath::inverse4x4(matrix));
  normalMatrix.set_col(3, LiteMath::float4());
  normalMatrix.set_row(3, LiteMath::float4());

  bool hasTang = !transformed.vTang4f.empty();

  for (size_t i = 0; i < transformed.VerticesNum(); ++i)
  {
    LiteMath::float4& vertex = (LiteMath::float4& )transformed.vPos4f[i * 4 + 0];
    vertex = matrix * vertex;
    vertex.w = 1.0f;

    LiteMath::float4& normal = (LiteMath::float4&)transformed.vNorm4f[i * 4 + 0];   
    normal = LiteMath::normalize3(normalMatrix * normal);
    normal.w = 1.0f;

    if (hasTang)
    {
      LiteMath::float4& tangent = (LiteMath::float4&)transformed.vTang4f[i * 4 + 0];
      tangent = LiteMath::normalize3(normalMatrix * tangent);
      tangent.w = 1.0f;
    }
  }

  return transformed;
}

void mergeMeshIntoMesh(cmesh::SimpleMesh& meshTo, const cmesh::SimpleMesh& meshFrom)
{
  assert(meshTo.topology == meshFrom.topology);

  auto oldVertNum = meshTo.VerticesNum();
  auto oldIdxNum  = meshTo.IndicesNum();
  auto oldTriNum  = meshTo.IndicesNum() / meshTo.PolySize();

  meshTo.Resize(oldVertNum + meshFrom.VerticesNum(), oldIdxNum + meshFrom.IndicesNum());

  memcpy(&meshTo.vPos4f[oldVertNum * 4],      meshFrom.vPos4f.data(),      sizeof(meshFrom.vPos4f[0]) * meshFrom.vPos4f.size());
  memcpy(&meshTo.vNorm4f[oldVertNum * 4],     meshFrom.vNorm4f.data(),     sizeof(meshFrom.vNorm4f[0]) * meshFrom.vNorm4f.size());
  memcpy(&meshTo.vTang4f[oldVertNum * 4],     meshFrom.vTang4f.data(),     sizeof(meshFrom.vTang4f[0]) * meshFrom.vTang4f.size());
  memcpy(&meshTo.vTexCoord2f[oldVertNum * 2], meshFrom.vTexCoord2f.data(), sizeof(meshFrom.vTexCoord2f[0]) * meshFrom.vTexCoord2f.size());

  memcpy(&meshTo.matIndices[oldTriNum], meshFrom.matIndices.data(), sizeof(meshFrom.matIndices[0]) * meshFrom.matIndices.size());
  memcpy(&meshTo.indices[oldIdxNum], meshFrom.indices.data(), sizeof(meshFrom.indices[0]) * meshFrom.indices.size());

  for (size_t i = oldIdxNum; i < meshTo.IndicesNum(); ++i)
  {
    meshTo.indices[i] += oldVertNum;
  }

}