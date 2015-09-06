#include "PolygonMesh.h"
#include "Shaders.h"
#include "MaterialManager.h"
#include "MaterialShader.h"
#include "ShadowMapBuildShader.h"
#include "MaterialGBufferShader.h"
#include "MaterialLightShader.h"
#include "MaterialTessellationShader.h"
#include "MaterialLightTessellationShader.h"
using ZShadeSandboxMesh::PolygonMesh;
//==================================================================================================================================
//==================================================================================================================================
PolygonMesh::PolygonMesh(ZShadeSandboxMath::Polygon* poly, ZShadeSandboxLighting::ShaderMaterial* material)
:	mPolygon(poly)
,	mMaterial(material)
,	m_Wireframe(false)
{
}
//==================================================================================================================================
void PolygonMesh::CreateSubsets(vector<int> subsetStartIndex)
{
	if (subsetStartIndex.size() == 0) return;
	
	Group initialG;
	initialG.IndexStart = subsetStartIndex[0];
	initialG.IndexCount = mPolygon->Attributes()->mIndexCount;
	mGroups.push_back(initialG);
	
	for (int i = 1; i < subsetStartIndex.size(); i++)
	{
		Group g;
		g.IndexStart = subsetStartIndex[i];
		g.IndexCount = mPolygon->Attributes()->mIndexCount - mGroups.back().IndexStart;
		mGroups.push_back(g);
	}
}
//==================================================================================================================================
XMFLOAT3& PolygonMesh::Position()
{
	return mPolygon->Attributes()->mPosition;
}
//==================================================================================================================================
XMFLOAT3& PolygonMesh::Scale()
{
	return mPolygon->Attributes()->mScale;
}
//==================================================================================================================================
XMFLOAT3& PolygonMesh::Rotate()
{
	return mPolygon->Attributes()->mRotation;
}
//==================================================================================================================================
void PolygonMesh::BuildIndexBuffer()
{
	mPolygon->Attributes()->BuildIndexBuffer();
}
//==================================================================================================================================
void PolygonMesh::BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type type)
{
	mPolygon->Attributes()->BuildVertexBuffer(type);
}
//==================================================================================================================================
void PolygonMesh::ReBuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type type)
{
	mPolygon->Attributes()->ReBuildVertexBuffer(type);
}
//==================================================================================================================================
void PolygonMesh::Render(ZShadeSandboxMesh::MeshRenderParameters mrp)
{
	// Cannot do anything if there is no camera
	if (mrp.camera == 0) return;
	
	if (!mrp.specifyWorld) mrp.world = mPolygon->Attributes()->WorldXM();
	
	if (mrp.useInstancing)
	{
		mPolygon->Attributes()->SetBuffersInstanced(mrp.renderType);
		mrp.world = XMMatrixIdentity();
	}
	else
	{
		mPolygon->Attributes()->SetBuffers(mrp.renderType);
	}
	
	if (mrp.reflection)
		mrp.view = mrp.camera->ReflectionView4x4();
	else
		mrp.view = mrp.camera->View4x4();
	
	if (mGroups.size() > 0)
	{
		for (auto& group : mGroups)
		{
			if (mrp.renderDeferred)
			{
				ZShadeSandboxShader::Shaders::pMaterialGBufferShader->Wireframe() = m_Wireframe;
				ZShadeSandboxShader::Shaders::pMaterialGBufferShader->Render11(
					group.IndexStart,
					group.IndexCount,
					mPolygon->Attributes()->mInstanceCount,
					mrp,
					mMaterial
				);
			}
			else
			{
				// Create a shadow map of the mesh
				if (mrp.shadowMap)
				{
					ZShadeSandboxShader::Shaders::pShadowMapBuildShader->Render(
						group.IndexStart,
						group.IndexCount,
						mPolygon->Attributes()->mInstanceCount,
						mrp.world,
						mrp.light->Perspective()
					);
				}
				else
				{
					if (mrp.tessellate)
					{
						// Assign the tessellation attributes to the material
						mMaterial->bEnableDistTess = mrp.enableDistTess;
						mMaterial->fMinTessDist = mrp.minTessDist;
						mMaterial->fMaxTessDist = mrp.maxTessDist;
						mMaterial->fMinTessFactor = mrp.minTess;
						mMaterial->fMaxTessFactor = mrp.maxTess;
						mMaterial->fNoDistTessFactor = mrp.noDistTessFactor;
						
						switch (mrp.renderType)
						{
							case ZShadeSandboxMesh::ERenderType::e3ControlPointPatchList:
							{
								if (mrp.renderLight)
								{
									ZShadeSandboxShader::Shaders::pTriMaterialLightTessellationShader->Wireframe() = m_Wireframe;
									ZShadeSandboxShader::Shaders::pTriMaterialLightTessellationShader->Render11(
										group.IndexStart,
										group.IndexCount,
										mPolygon->Attributes()->mInstanceCount,
										mrp,
										mMaterial
									);
								}
								else
								{
									ZShadeSandboxShader::Shaders::pTriMaterialTessellationShader->Wireframe() = m_Wireframe;
									ZShadeSandboxShader::Shaders::pTriMaterialTessellationShader->Render11(
										group.IndexStart,
										group.IndexCount,
										mPolygon->Attributes()->mInstanceCount,
										mrp,
										mMaterial
									);
								}
							}
							break;
							case ZShadeSandboxMesh::ERenderType::e4ControlPointPatchList:
							{
								if (mrp.renderLight)
								{
									ZShadeSandboxShader::Shaders::pQuadMaterialLightTessellationShader->Wireframe() = m_Wireframe;
									ZShadeSandboxShader::Shaders::pQuadMaterialLightTessellationShader->Render11(
										group.IndexStart,
										group.IndexCount,
										mPolygon->Attributes()->mInstanceCount,
										mrp,
										mMaterial
									);
								}
								else
								{
									ZShadeSandboxShader::Shaders::pQuadMaterialTessellationShader->Wireframe() = m_Wireframe;
									ZShadeSandboxShader::Shaders::pQuadMaterialTessellationShader->Render11(
										group.IndexStart,
										group.IndexCount,
										mPolygon->Attributes()->mInstanceCount,
										mrp,
										mMaterial
									);
								}
							}
							break;
							default:
							{
								//The shader was not defined
								ZShadeMessageCenter::MsgBoxError(NULL, "CustomMesh: Tessellation Shader was not defined !!!");
							}
							break;
						}
					}
					else
					{
						if (mrp.renderLight)
						{
							ZShadeSandboxShader::Shaders::pMaterialLightShader->Wireframe() = m_Wireframe;
							ZShadeSandboxShader::Shaders::pMaterialLightShader->Render11(
								group.IndexStart,
								group.IndexCount,
								mPolygon->Attributes()->mInstanceCount,
								mrp,
								mMaterial
							);
						}
						else
						{
							ZShadeSandboxShader::Shaders::pMaterialShader->Wireframe() = m_Wireframe;
							ZShadeSandboxShader::Shaders::pMaterialShader->Render11(
								group.IndexStart,
								group.IndexCount,
								mPolygon->Attributes()->mInstanceCount,
								mrp,
								mMaterial
							);
						}
					}
				}
			}
		}
	}
	else
	{
		if (mrp.renderDeferred)
		{
			ZShadeSandboxShader::Shaders::pMaterialGBufferShader->Wireframe() = m_Wireframe;
			ZShadeSandboxShader::Shaders::pMaterialGBufferShader->Render11(
				mPolygon->GetIndexCount(),
				mPolygon->Attributes()->mInstanceCount,
				mrp,
				mMaterial
			);
			
			return;
		}
		
		// Create a shadow map of the mesh
		if (mrp.shadowMap)
		{
			ZShadeSandboxShader::Shaders::pShadowMapBuildShader->Render(
				mPolygon->GetIndexCount(),
				mPolygon->Attributes()->mInstanceCount,
				mrp.world,
				mrp.light->Perspective()
			);
			
			return;
		}
		
		if (mrp.tessellate)
		{
			// Assign the tessellation attributes to the material
			mMaterial->bEnableDistTess = mrp.enableDistTess;
			mMaterial->fMinTessDist = mrp.minTessDist;
			mMaterial->fMaxTessDist = mrp.maxTessDist;
			mMaterial->fMinTessFactor = mrp.minTess;
			mMaterial->fMaxTessFactor = mrp.maxTess;
			mMaterial->fNoDistTessFactor = mrp.noDistTessFactor;
			
			switch (mrp.renderType)
			{
				case ZShadeSandboxMesh::ERenderType::e3ControlPointPatchList:
				{
					if (mrp.renderLight)
					{
						ZShadeSandboxShader::Shaders::pTriMaterialLightTessellationShader->Wireframe() = m_Wireframe;
						ZShadeSandboxShader::Shaders::pTriMaterialLightTessellationShader->Render11(
							mPolygon->GetIndexCount(),
							mPolygon->Attributes()->mInstanceCount,
							mrp,
							mMaterial
						);
					}
					else
					{
						ZShadeSandboxShader::Shaders::pTriMaterialTessellationShader->Wireframe() = m_Wireframe;
						ZShadeSandboxShader::Shaders::pTriMaterialTessellationShader->Render11(
							mPolygon->GetIndexCount(),
							mPolygon->Attributes()->mInstanceCount,
							mrp,
							mMaterial
						);
					}
				}
				break;
				case ZShadeSandboxMesh::ERenderType::e4ControlPointPatchList:
				{
					if (mrp.renderLight)
					{
						ZShadeSandboxShader::Shaders::pQuadMaterialLightTessellationShader->Wireframe() = m_Wireframe;
						ZShadeSandboxShader::Shaders::pQuadMaterialLightTessellationShader->Render11(
							mPolygon->GetIndexCount(),
							mPolygon->Attributes()->mInstanceCount,
							mrp,
							mMaterial
						);
					}
					else
					{
						ZShadeSandboxShader::Shaders::pQuadMaterialTessellationShader->Wireframe() = m_Wireframe;
						ZShadeSandboxShader::Shaders::pQuadMaterialTessellationShader->Render11(
							mPolygon->GetIndexCount(),
							mPolygon->Attributes()->mInstanceCount,
							mrp,
							mMaterial
						);
					}
				}
				break;
				default:
				{
					//The shader was not defined
					ZShadeMessageCenter::MsgBoxError(NULL, "CustomMesh: Tessellation Shader was not defined !!!");
				}
				break;
			}
		}
		else
		{
			if (mrp.renderLight)
			{
				ZShadeSandboxShader::Shaders::pMaterialLightShader->Wireframe() = m_Wireframe;
				ZShadeSandboxShader::Shaders::pMaterialLightShader->Render11(
					mPolygon->GetIndexCount(),
					mPolygon->Attributes()->mInstanceCount,
					mrp,
					mMaterial
				);
			}
			else
			{
				ZShadeSandboxShader::Shaders::pMaterialShader->Wireframe() = m_Wireframe;
				ZShadeSandboxShader::Shaders::pMaterialShader->Render11(
					mPolygon->GetIndexCount(),
					mPolygon->Attributes()->mInstanceCount,
					mrp,
					mMaterial
				);
			}
		}
	}
}
//==================================================================================================================================