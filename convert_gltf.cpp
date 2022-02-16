#include <string>
#include <iostream>

#define TINYGLTF_IMPLEMENTATION
//#define TINYGLTF_NO_STB_IMAGE_WRITE
//#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
//#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_USE_CPP14

#include "convert_gltf.h"
#include "gltf_utils.h"
#include "image_loader.h"
#include "HydraAPI.h"
#include "HydraXMLHelpers.h"

void load_gltf_nodes_recursive(HRSceneInstRef a_scnRef, const tinygltf::Model &a_model, const tinygltf::Node& a_node,
                               const LiteMath::float4x4& a_parentMatrix,
                               std::unordered_map<int, HRMeshRef> &a_loadedMeshesToMeshId)
{
  auto nodeMatrix = a_parentMatrix * transformMatrixFromGLTFNode(a_node);

  for (size_t i = 0; i < a_node.children.size(); i++)
  {
    load_gltf_nodes_recursive(a_scnRef, a_model, a_model.nodes[a_node.children[i]], nodeMatrix, a_loadedMeshesToMeshId);
  }

  if(a_node.mesh > -1)
  {
    if(!a_loadedMeshesToMeshId.count(a_node.mesh))
    {
      const tinygltf::Mesh mesh = a_model.meshes[a_node.mesh];
      auto simpleMesh           = simpleMeshFromGLTFMesh(a_model, mesh);

      if(simpleMesh.VerticesNum() > 0)
      {
        auto mesh_name = s2ws(mesh.name);

        HRMeshRef meshRef = hrMeshCreate(mesh_name.c_str());
        hrMeshOpen(meshRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
        {
          hrMeshVertexAttribPointer4f(meshRef, L"pos",      simpleMesh.vPos4f.data());
          hrMeshVertexAttribPointer4f(meshRef, L"norm",     simpleMesh.vNorm4f.data());
          hrMeshVertexAttribPointer2f(meshRef, L"texcoord", simpleMesh.vTexCoord2f.data());

          const int* mat_ind = (const int *)(simpleMesh.matIndices.data());
          hrMeshPrimitiveAttribPointer1i(meshRef, L"mind", mat_ind);

          const int* ind = (const int *)(simpleMesh.indices.data());
          hrMeshAppendTriangles3(meshRef, int(simpleMesh.indices.size()), ind);
        }
        hrMeshClose(meshRef);

        a_loadedMeshesToMeshId[a_node.mesh] = meshRef;

        std::cout << "Loading meshes # " << meshRef.id << "\r";
      }
    }
    hrMeshInstance(a_scnRef, a_loadedMeshesToMeshId[a_node.mesh], nodeMatrix.L());
  }
}

bool load_scene_gltf(const char* in_path, HRSceneInstRef scnRef)
{
  std::string scenePath(in_path);
  tinygltf::Model gltfModel;
  tinygltf::TinyGLTF gltfContext;
  std::string error, warning;

  std::string sceneFolder;
  auto found = scenePath.find_last_of('/');
  if(found != std::string::npos && found != scenePath.size())
    sceneFolder = scenePath.substr(0, found + 1);
  else
    sceneFolder = "./";

  bool loaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, scenePath);

  if(!loaded)
  {
    std::cout << "Cannot load glTF scene from: " << scenePath;
    return false;
  }

  const tinygltf::Scene& scene = gltfModel.scenes[0];

  // geometry and instancing
  {
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    std::unordered_map<int, HRMeshRef> loaded_meshes_to_meshId;
    for(size_t i = 0; i < scene.nodes.size(); ++i)
    {
      const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
      auto identity = LiteMath::float4x4();
      load_gltf_nodes_recursive(scnRef, gltfModel, node, identity, loaded_meshes_to_meshId);
    }
    std::cout << std::endl;
  }

  // textures
//  std::vector<ImageFileInfo> textureInfos;
  {
//    textureInfos.reserve(gltfModel.materials.size() * 4);
    for (tinygltf::Image &image : gltfModel.images)
    {
      auto texturePath      = sceneFolder + image.uri;
      ImageFileInfo texInfo = getImageInfo(texturePath);
      if(!texInfo.is_ok)
      {
        std::cout << "Texture at \"" << texturePath << "\" is absent or corrupted." ;
      }
//      textureInfos.push_back(texInfo);
      auto texturePath_w = s2ws(texturePath);
      auto tex_ref = hrTexture2DCreateFromFile(texturePath_w.c_str());

      std::cout << "Loading textures # " << tex_ref.id << "\r";
    }
    std::cout << std::endl;
  }

  // materials
//  std::vector<MaterialData_pbrMR> materials;
  {
//    materials.reserve(gltfModel.materials.size());
    for(const tinygltf::Material &gltfMat : gltfModel.materials)
    {
      constexpr int32_t HAPI_TEX_ID_OFFSET = 1; //HydraAPI reserves texture id = 0 for special texture
      MaterialData_pbrMR mat = materialDataFromGLTF(gltfMat, HAPI_TEX_ID_OFFSET);
//      materials.push_back(mat);

      auto mat_name = s2ws(gltfMat.name);
      HRMaterialRef matRef = hrMaterialCreate(mat_name.c_str());
      hrMaterialOpen(matRef, HR_WRITE_DISCARD);
      {
        auto matNode = hrMaterialParamNode(matRef);

        // base color
        {
          auto base = matNode.append_child(L"base_color");
          HydraXMLHelpers::WriteFloat4(base.append_child(L"color").append_attribute(L"val"), mat.baseColor);

          if (mat.baseColorTexId > 0)
          {
            HRTextureNodeRef tex;
            tex.id = mat.baseColorTexId;

            hrTextureBind(tex, base.child(L"color"));
          }
        }

        // emission
        {
          auto emission = matNode.append_child(L"emission");
          HydraXMLHelpers::WriteFloat3(emission.append_child(L"color").append_attribute(L"val"), mat.emissionColor);

          if (mat.emissionTexId > 0)
          {
            HRTextureNodeRef tex;
            tex.id = mat.emissionTexId;

            hrTextureBind(tex, emission.child(L"color"));
          }
        }

        // metallic roughness
        {
          auto metallic = matNode.append_child(L"metallic");
          HydraXMLHelpers::WriteFloat(metallic.append_attribute(L"val"), mat.metallic);

          auto roughness = matNode.append_child(L"roughness");
          HydraXMLHelpers::WriteFloat(roughness.append_attribute(L"val"), mat.roughness);

          if (mat.metallicRoughnessTexId > 0)
          {
            HRTextureNodeRef tex;
            tex.id = mat.metallicRoughnessTexId;

            hrTextureBind(tex, metallic);
            hrTextureBind(tex, roughness);
          }
        }

        // opacity
        {
          auto opacity = matNode.append_child(L"opacity");
          HydraXMLHelpers::WriteFloat(opacity.append_child(L"alphaCutoff").append_attribute(L"val"), mat.alphaCutoff);
          opacity.append_child(L"alphaMode").append_attribute(L"val").set_value(gltfMat.alphaMode.c_str());

          if (mat.occlusionTexId > 0)
          {
            HRTextureNodeRef tex;
            tex.id = mat.occlusionTexId;
            hrTextureBind(tex, opacity);
          }
        }

        // normal map
        if (mat.normalTexId > 0)
        {
          auto normal_map = matNode.append_child(L"normal_map");
          HRTextureNodeRef tex;
          tex.id = mat.normalTexId;
          hrTextureBind(tex, normal_map);
        }
      }
      hrMaterialClose(matRef);

      std::cout << "Loading materials # " << matRef.id << "\r";
    }
    std::cout << std::endl;
  }

  return true;
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

    node.append_child(L"width").text()  = 1024;
    node.append_child(L"height").text() = 1024;

    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";

    node.append_child(L"trace_depth").text()      = 6;
    node.append_child(L"diff_trace_depth").text() = 3;
    node.append_child(L"maxRaysPerPixel").text()  = 512;
    node.append_child(L"resources_path").text()   = L"..";
    node.append_child(L"offline_pt").text()       = 0;
  }
  hrRenderClose(renderRef);

  return renderRef;
}


bool convert_gltf_to_hydra(const char* src_path, const char* dest_path)
{
  hrErrorCallerPlace(L"convert_gltf_to_hydra");

  std::string out(dest_path);
  auto hydra_out = s2ws(out);
  hr_fs::mkdir(dest_path);

  hrSceneLibraryOpen(hydra_out.c_str(), HR_WRITE_DISCARD);

  HRSceneInstRef scnRef = hrSceneCreate(L"exported_scene");

  load_scene_gltf(src_path, scnRef);

  add_default_light(scnRef);
  auto camRef    = add_default_camera();

  HRRenderRef renderRef;
  renderRef.id = -1;
  //  auto renderRef = add_default_render(0);

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef, camRef);

  hrSceneLibraryClose();

  return true;
}
