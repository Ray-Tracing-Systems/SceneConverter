#ifndef CHIMERA_GLTF_UTILS_H
#define CHIMERA_GLTF_UTILS_H

#include <LiteMath.h>
#include "cmesh.h"
#include "tinygltf/tiny_gltf.h"

struct MaterialData_pbrMR
{
  LiteMath::float4 baseColor {1.0f, 0.0f, 1.0f, 1.0f};

  float metallic               = 0.0f;
  float roughness              = 0.0f;
  int   baseColorTexId         = -1;
  int   metallicRoughnessTexId = -1;

  LiteMath::float3 emissionColor {0.0f, 0.0f, 0.0f};
  int emissionTexId = -1;

  int   normalTexId    = -1;
  int   occlusionTexId = -1;
  float alphaCutoff    = 0.0f;
  int   alphaMode      = 0;
};

void getNumVerticesAndIndicesFromGLTFMesh(const tinygltf::Model &a_model, const tinygltf::Mesh &a_mesh, uint32_t& numVertices, uint32_t& numIndices);
cmesh::SimpleMesh  simpleMeshFromGLTFMesh(const tinygltf::Model &a_model, const tinygltf::Mesh &a_mesh);
LiteMath::float4x4 transformMatrixFromGLTFNode(const tinygltf::Node &node);
MaterialData_pbrMR materialDataFromGLTF(const tinygltf::Material &gltfMat, const std::vector<tinygltf::Texture> &gltfTextures,
                                        int32_t texIdOffset = 0);

#endif// CHIMERA_GLTF_UTILS_H
