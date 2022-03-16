#include <string>
#include <iostream>

#define TINYGLTF_IMPLEMENTATION
//#define TINYGLTF_NO_STB_IMAGE_WRITE
//#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
//#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_USE_CPP14

#include "convert_common.h"
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

void collapse_gltf_nodes_recursive(cmesh::SimpleMesh& outMesh, const tinygltf::Model& a_model, const tinygltf::Node& a_node, const LiteMath::float4x4& a_parentMatrix,
  std::unordered_map<int, cmesh::SimpleMesh>& a_idToSimpleMesh, uint32_t &idx)
{
  auto nodeMatrix = a_parentMatrix * transformMatrixFromGLTFNode(a_node);

  for (size_t i = 0; i < a_node.children.size(); i++)
  {
    collapse_gltf_nodes_recursive(outMesh, a_model, a_model.nodes[a_node.children[i]], nodeMatrix, a_idToSimpleMesh, idx);
  }

  if (a_node.mesh > -1)
  {
    if (!a_idToSimpleMesh.count(a_node.mesh))
    {
      const tinygltf::Mesh mesh = a_model.meshes[a_node.mesh];
      a_idToSimpleMesh[a_node.mesh] = simpleMeshFromGLTFMesh(a_model, mesh);
    }
    
    std::cout << "Merging part # " << idx << "\r";
    mergeMeshIntoMesh(outMesh, transformSimpleMesh(a_idToSimpleMesh[a_node.mesh], nodeMatrix));

    idx++;
  }
}

void gltf_mat_to_xml(HRMaterialRef matRef, const MaterialData_pbrMR &mat, const tinygltf::Material &gltfMat)
{
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
}

void gltf_mat_to_hydra(HRMaterialRef matRef, const MaterialData_pbrMR &mat, const tinygltf::Material &gltfMat)
{
  hrMaterialOpen(matRef, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRef);

    // diffuse
    {
      auto diffuse = matNode.append_child(L"diffuse");
      HydraXMLHelpers::WriteFloat4(diffuse.append_child(L"color").append_attribute(L"val"), mat.baseColor);

      if (mat.baseColorTexId > 0)
      {
        HRTextureNodeRef tex;
        tex.id = mat.baseColorTexId;

        auto texNode = hrTextureBind(tex, diffuse.child(L"color"));
        texNode.append_attribute(L"matrix");
        float samplerMatrix[16] = { 1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1 };

        texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
        texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
        texNode.append_attribute(L"input_gamma").set_value(2.2f);
        texNode.append_attribute(L"input_alpha").set_value(L"rgb");

        HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
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

        auto texNode = hrTextureBind(tex, emission.child(L"color"));
        texNode.append_attribute(L"matrix");
        float samplerMatrix[16] = { 1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1 };

        texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
        texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
        texNode.append_attribute(L"input_gamma").set_value(2.2f);
        texNode.append_attribute(L"input_alpha").set_value(L"rgb");

        HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
      }
    }

    // reflectivity
    {
      auto refl = matNode.append_child(L"reflectivity");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      HydraXMLHelpers::WriteFloat(refl.append_child(L"glossiness").append_attribute(L"val"), 1.0f - mat.roughness);

      float ior = 1.0f;
      if(mat.metallic > LiteMath::EPSILON)
        ior = (1 + sqrtf(mat.metallic)) / (1 - sqrtf(mat.metallic));

      HydraXMLHelpers::WriteFloat(refl.append_child(L"fresnel_ior").append_attribute(L"val"), ior);
      HydraXMLHelpers::WriteFloat(refl.append_child(L"fresnel").append_attribute(L"val"), 1);

      if (mat.metallicRoughnessTexId > 0)
      {
        HRTextureNodeRef tex;
        tex.id = mat.metallicRoughnessTexId;

        auto texNode = hrTextureBind(tex, refl.child(L"glossiness"));
        texNode.append_attribute(L"matrix");
        float samplerMatrix[16] = { 1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1 };

        texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
        texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
        texNode.append_attribute(L"input_gamma").set_value(2.2f);
        texNode.append_attribute(L"input_alpha").set_value(L"rgb");

        HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
      }
    }

    // opacity
    {
      auto opacity = matNode.append_child(L"opacity");
//      HydraXMLHelpers::WriteFloat(opacity.append_child(L"alphaCutoff").append_attribute(L"val"), mat.alphaCutoff);
//      opacity.append_child(L"alphaMode").append_attribute(L"val").set_value(gltfMat.alphaMode.c_str());

      if (mat.occlusionTexId > 0)
      {
        HRTextureNodeRef tex;
        tex.id = mat.occlusionTexId;
        auto texNode = hrTextureBind(tex, opacity);
        texNode.append_attribute(L"matrix");
        float samplerMatrix[16] = { 1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1 };

        texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
        texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
        texNode.append_attribute(L"input_gamma").set_value(2.2f);
        texNode.append_attribute(L"input_alpha").set_value(L"rgb");

        HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
      }
    }

    // normal map
    if (mat.normalTexId > 0)
    {
      auto displ = matNode.append_child(L"displacement");
      displ.append_attribute(L"type").set_value(L"normal_bump");
      HRTextureNodeRef tex;
      tex.id = mat.normalTexId;
      auto texNode = hrTextureBind(tex, displ.append_child(L"normal_map"));
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    }
  }
  hrMaterialClose(matRef);
}

bool load_scene_gltf(const std::filesystem::path& in_path, const std::filesystem::path& dest_path, HRSceneInstRef scnRef,
  bool export_single_mesh, bool convert_materials)
{
  tinygltf::Model gltfModel;
  tinygltf::TinyGLTF gltfContext;
  std::string error, warning;

  bool loaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, in_path.string());

  if(!loaded)
  {
    std::cout << "Cannot load glTF scene from: " << in_path;
    return false;
  }

  const tinygltf::Scene& scene = gltfModel.scenes[0];

  // geometry and instancing
  HRMeshRef singleMesh;
  
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  if (export_single_mesh)
  {
    cmesh::SimpleMesh outMesh;
    auto identity = LiteMath::float4x4();
    std::unordered_map<int, cmesh::SimpleMesh> idToSimpleMesh; 
    uint32_t mesh_idx = 0;
    for (size_t i = 0; i < scene.nodes.size(); ++i)
    {
      const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
      collapse_gltf_nodes_recursive(outMesh, gltfModel, node, identity, idToSimpleMesh, mesh_idx);
    }
    std::cout << std::endl;

    if (outMesh.VerticesNum() > 0)
    {

      singleMesh = hrMeshCreate(L"collapsed_mesh");
      hrMeshOpen(singleMesh, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
      {
        hrMeshVertexAttribPointer4f(singleMesh, L"pos", outMesh.vPos4f.data());
        hrMeshVertexAttribPointer4f(singleMesh, L"norm", outMesh.vNorm4f.data());
        hrMeshVertexAttribPointer2f(singleMesh, L"texcoord", outMesh.vTexCoord2f.data());

        const int* mat_ind = (const int*)(outMesh.matIndices.data());
        hrMeshPrimitiveAttribPointer1i(singleMesh, L"mind", mat_ind);

        const int* ind = (const int*)(outMesh.indices.data());
        hrMeshAppendTriangles3(singleMesh, int(outMesh.indices.size()), ind);
      }
      hrMeshClose(singleMesh);

      hrMeshInstance(scnRef, singleMesh, identity.L());
    }
  }
  else
  {
    std::unordered_map<int, HRMeshRef> loaded_meshes_to_meshId;
    const auto identity = LiteMath::float4x4();
    for (size_t i = 0; i < scene.nodes.size(); ++i)
    {
      const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
      load_gltf_nodes_recursive(scnRef, gltfModel, node, identity, loaded_meshes_to_meshId);
    }
    std::cout << std::endl;
  }

  // textures
  {
    for (tinygltf::Image& image : gltfModel.images)
    {
      std::filesystem::path texturePath = in_path.parent_path();
      texturePath.append(image.uri);
      ImageFileInfo texInfo = getImageInfo(texturePath.string());
      if (!texInfo.is_ok)
      {
        std::cout << "Texture at \"" << texturePath << "\" is absent or corrupted.";
      }

      auto tex_ref = hrTexture2DCreateFromFile(texturePath.wstring().c_str());

      std::cout << "Loading textures # " << tex_ref.id << "\r";
    }
    std::cout << std::endl;
  }

  // materials
  {
    {
      HRMaterialRef matRef = hrMaterialCreate(L"No material");
      hrMaterialOpen(matRef, HR_WRITE_DISCARD);
      auto matNode = hrMaterialParamNode(matRef);

      auto diffuse = matNode.append_child(L"diffuse");
      auto gray = LiteMath::make_float4(0.5f, 0.5f, 0.5f, 1.0f);
      HydraXMLHelpers::WriteFloat4(diffuse.append_child(L"color").append_attribute(L"val"), gray);

      hrMaterialClose(matRef);
    }

    for (const tinygltf::Material& gltfMat : gltfModel.materials)
    {
      MaterialData_pbrMR mat = materialDataFromGLTF(gltfMat, gltfModel.textures, HAPI_TEX_ID_OFFSET);

      auto mat_name = s2ws(gltfMat.name);
      HRMaterialRef matRef = hrMaterialCreate(mat_name.c_str());
      if (convert_materials)
        gltf_mat_to_hydra(matRef, mat, gltfMat);
      else
        gltf_mat_to_xml(matRef, mat, gltfMat);

      std::cout << "Loading materials # " << matRef.id << "\r";
    }
    std::cout << std::endl;
  }

  if(export_single_mesh)
    saveMesh(singleMesh, dest_path, in_path.stem());
  

  return true;
}


bool convert_gltf_to_hydra(const std::filesystem::path& src_path, const std::filesystem::path& dest_path,
  bool export_single_mesh, bool convert_materials)
{
  hrErrorCallerPlace(L"convert_gltf_to_hydra");

  auto sceneLib = dest_path;
  sceneLib.append("scenelib");
  std::filesystem::create_directories(sceneLib);

  hrSceneLibraryOpen(sceneLib.wstring().c_str(), HR_WRITE_DISCARD);

  HRSceneInstRef scnRef = hrSceneCreate(L"exported_scene");

  load_scene_gltf(src_path, dest_path, scnRef, export_single_mesh, convert_materials);

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
