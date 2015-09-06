#include "FBXMesh.h"
#include "Vertex.h"
#include "Shaders.h"
#include "MaterialShader.h"
#include "ShadowMapBuildShader.h"
#include "MaterialGBufferShader.h"
#include "MaterialLightShader.h"
#include "MaterialTessellationShader.h"
#include "MaterialLightTessellationShader.h"
#include "MaterialManager.h"
using ZShadeSandboxMesh::FBXMesh;
//===============================================================================================================================
//===============================================================================================================================
FBXMesh::FBXMesh(D3D* d3d, GameDirectory3D* gd3d)
:	mD3DSystem(d3d)
,	mGD3D(gd3d)
,	bWireframe(false)
,	fFarPlane(1000)
{
	mLoader = new FBXLoader(mD3DSystem, mGD3D);
}
//===============================================================================================================================
void FBXMesh::Scale(XMFLOAT3 v)
{
	mAttributes->mScale = v;
}
//===============================================================================================================================
void FBXMesh::Rotate(XMFLOAT3 v)
{
	mAttributes->mRotation = v;
}
//===============================================================================================================================
void FBXMesh::Position(XMFLOAT3 v)
{
	mAttributes->mPosition = v;
}
//===============================================================================================================================
void FBXMesh::Load(string filename, bool flipAxisYZ, ZShadeSandboxMesh::MeshType::Type mt)
{
	mMeshType = mt;
	
	mAttributes = new ZShadeSandboxMesh::MeshAttributes();
	mAttributes->mD3D = mD3DSystem;

	mLoader->Load(filename.c_str());
	
	for (int subsetID = 0; subsetID < mLoader->SubsetCount(); subsetID++)
	{
		int localVertexCount = mLoader->SubsetVertexCount(subsetID);
		for (int vertexID = 0; vertexID < localVertexCount; vertexID++)
		{
			ZShadeSandboxMesh::VertexNormalTex v;
			v.position = mLoader->SubsetVertices(subsetID)[vertexID].position;
			if (flipAxisYZ)
			{
				float temp = v.position.y;
				v.position.y = v.position.z;
				v.position.z = temp;
			}
			v.normal = mLoader->SubsetVertices(subsetID)[vertexID].normal;
			v.texture = mLoader->SubsetVertices(subsetID)[vertexID].texture;
			v.texture.y = 1.0f - v.texture.y;
			mAttributes->mVerticesNormalTex.push_back(v);
		}

		// Represents the start index of the current subset since it starts
		// at the end of the last subset.
		int startIndex = mAttributes->mIndices.size();

		int localIndexCount = mLoader->SubsetIndexCount(subsetID);
		for (int indexID = 0; indexID < localIndexCount; indexID++)
		{
			mAttributes->mIndices.push_back(mLoader->SubsetIndices(subsetID)[indexID]);
		}

		int materialID = mLoader->SubsetMaterialIndex(subsetID);
		string materialName = mLoader->SubsetMaterialName(subsetID);

		// Finally create the subset
		GroupSubset gs;
		gs.startIndex = startIndex;
		gs.indexCount = localIndexCount - ((mGroups.size() > 0) ? mGroups.back().startIndex : 0);
		gs.materialName = materialName;
		gs.material = mLoader->SubsetMaterial(subsetID, materialID);
		mGroups.push_back(gs);
	}
	
	if ((mAttributes->mIndices.size() % 3) == 0)
	{
		for (auto it = mAttributes->mIndices.begin(); it != mAttributes->mIndices.end(); it += 3)
		{
			std::swap(*it, *(it + 2));
		}
	}
	
	mAttributes->mVertexCount = mAttributes->mVerticesNormalTex.size();
	mAttributes->mIndexCount = mAttributes->mIndices.size();
	mAttributes->mTriangleCount = mAttributes->mIndexCount / 3;
	
	mAttributes->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::VT_NormalTex);
	mAttributes->BuildIndexBuffer();
	
	mMaterial = new ZShadeSandboxLighting::ShaderMaterial(mD3DSystem, "FarmhouseTexture");
	mMaterial->vAmbientColor = XMFLOAT4(1, 1, 1, 1);
	mMaterial->vDiffuseColor = XMFLOAT4(1, 1, 1, 1);
	mMaterial->vSpecularColor = XMFLOAT4(1, 1, 1, 1);
	mMaterial->vEmissiveColor = XMFLOAT4(1, 1, 1, 1);
	mMaterial->fSpecularPower = 5.0f;
	mMaterial->fSpecularIntensity = 0.3f;
	mMaterial->fAlpha = 0.0f;
	mMaterial->bHasTransparency = true;
	mMaterial->bEnableLighting = true;
	mMaterial->bBackfaceCull = false;
	mMaterial->fDetailBrightness = 1.8f;
	mMaterial->AddDiffuseTexture(mGD3D->m_textures_path, "texture.tga");
	mMaterial->AddDetailMapTexture(mGD3D->m_textures_path, "detail001.dds");
	mMaterial->AddNormalMapTexture(mGD3D->m_textures_path, "FarmhouseTextureBumpMap.dds");
	mMaterial->AddAlphaMapTexture(mGD3D->m_textures_path, "texture.tga");
}
//===============================================================================================================================
void FBXMesh::Render(ZShadeSandboxMesh::MeshRenderParameters mrp)
{
	if (mrp.camera == NULL || !mrp.toggleMesh) return;
	
	if (!mrp.specifyWorld) mrp.world = mAttributes->WorldXM();
	
	if (mrp.useInstancing)
	{
		mAttributes->SetBuffersInstanced(mrp.renderType);
	}
	else
	{
		mAttributes->SetBuffers(mrp.renderType);
	}
	
	if (mrp.reflection)
		mrp.view = mrp.camera->ReflectionView4x4();
	else
		mrp.view = mrp.camera->View4x4();
	
	for (auto& group : mGroups)
	{
		auto material = mMaterial;//group.material;
		
		material->fFarPlane = fFarPlane;
		
		if (material->bHasTransparency)
		{
			mD3DSystem->TurnOnAlphaBlending();
		}
		
		if (mrp.renderDeferred)
		{
			ZShadeSandboxShader::Shaders::pMaterialGBufferShader->Wireframe() = bWireframe;
			ZShadeSandboxShader::Shaders::pMaterialGBufferShader->Render11(
				group.startIndex,
				group.indexCount,
				mAttributes->mInstanceCount,
				mrp,
				material
			);
		}
		else
		{
			if (mrp.tessellate)
			{
				// Assign the tessellation attributes to the material
				material->bEnableDistTess = mrp.enableDistTess;
				material->fMinTessDist = mrp.minTessDist;
				material->fMaxTessDist = mrp.maxTessDist;
				material->fMinTessFactor = mrp.minTess;
				material->fMaxTessFactor = mrp.maxTess;
				material->fNoDistTessFactor = mrp.noDistTessFactor;
				
				switch (mrp.renderType)
				{
					case ZShadeSandboxMesh::ERenderType::e3ControlPointPatchList:
					{
						if (mrp.renderLight)
						{
							ZShadeSandboxShader::Shaders::pTriMaterialLightTessellationShader->Wireframe() = bWireframe;
							ZShadeSandboxShader::Shaders::pTriMaterialLightTessellationShader->Render11(
								group.startIndex,
								group.indexCount,
								mAttributes->mInstanceCount,
								mrp,
								material
							);
						}
						else
						{
							ZShadeSandboxShader::Shaders::pTriMaterialTessellationShader->Wireframe() = bWireframe;
							ZShadeSandboxShader::Shaders::pTriMaterialTessellationShader->Render11(
								group.startIndex,
								group.indexCount,
								mAttributes->mInstanceCount,
								mrp,
								material
							);
						}
					}
					break;
					case ZShadeSandboxMesh::ERenderType::e4ControlPointPatchList:
					{
						if (mrp.renderLight)
						{
							ZShadeSandboxShader::Shaders::pQuadMaterialLightTessellationShader->Wireframe() = bWireframe;
							ZShadeSandboxShader::Shaders::pQuadMaterialLightTessellationShader->Render11(
								group.startIndex,
								group.indexCount,
								mAttributes->mInstanceCount,
								mrp,
								material
							);
						}
						else
						{
							ZShadeSandboxShader::Shaders::pQuadMaterialTessellationShader->Wireframe() = bWireframe;
							ZShadeSandboxShader::Shaders::pQuadMaterialTessellationShader->Render11(
								group.startIndex,
								group.indexCount,
								mAttributes->mInstanceCount,
								mrp,
								material
							);
						}
					}
					break;
					default:
					{
						//The shader was not defined
						ZShadeMessageCenter::MsgBoxError(NULL, "FBXSkinnedMesh: Tessellation Shader was not defined !!!");
					}
					break;
				}
			}
			else
			{
				ZShadeSandboxShader::Shaders::pMaterialShader->Wireframe() = bWireframe;
				ZShadeSandboxShader::Shaders::pMaterialShader->Render11(
					group.startIndex,
					group.indexCount,
					mAttributes->mInstanceCount,
					mrp,
					material
				);
			}
			
			if (material->bHasTransparency)
			{
				mD3DSystem->TurnOffAlphaBlending();
			}
		}
	}
}
//===============================================================================================================================