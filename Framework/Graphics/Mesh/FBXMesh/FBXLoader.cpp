#include "FBXLoader.h"
#include <algorithm>
#include "ZShadeMessageCenter.h"
#include "FBXHelper.h"
#include "Convert.h"
#include "MaterialManager.h"
using ZShadeSandboxMesh::FBXLoader;
// https://github.com/lang1991/FBXExporter/blob/master/FBXExporter/FBXExporter.cpp
//===============================================================================================================================
//===============================================================================================================================
int FBXLoader::iFbxMaterialCounter = 0;
//===============================================================================================================================
FBXLoader::FBXLoader(D3D* d3d, GameDirectory3D* gd3d)
:	m_pD3DSystem(d3d)
,	m_pGD3D(gd3d)
,   m_pFBXManager(nullptr)
,	m_pFbxScene(nullptr)
//,	mTriangleCount(0)
,	bHasAnimation(true)
,	iCurrentSubset(0)
{
	QueryPerformanceFrequency(&mCPUFreq);
}
//===============================================================================================================================
FBXLoader::~FBXLoader()
{
}
//===============================================================================================================================
int FBXLoader::SubsetVertexCount(int subsetID)
{
	FBXSubsets* subset = mSubsets[subsetID];
	
	if (subset == 0) return 0;
	
	return subset->mVertices.size();
}
//===============================================================================================================================
int FBXLoader::SubsetIndexCount(int subsetID)
{
	FBXSubsets* subset = mSubsets[subsetID];
	
	if (subset == 0) return 0;
	
	//int count = 0;
	//for (int i = 0; i < subset->mTriangles.size(); i++)
	//{
	//	count += subset->mTriangles[i].indices.size();
	//}
	
	return subset->mIndices.size();
}
//===============================================================================================================================
int FBXLoader::SubsetPolygonCount(int subsetID)
{
	FBXSubsets* subset = mSubsets[subsetID];
	
	if (subset == 0) return 0;
	
	//return subset->mTriangles.size();
	return subset->mPolygonCount;
}
//===============================================================================================================================
int FBXLoader::SubsetMaterialIndex(int subsetID)
{
	FBXSubsets* subset = mSubsets[subsetID];
	
	if (subset == 0) return 0;
	
	return subset->mMaterialIndex;
}
//===============================================================================================================================
string FBXLoader::SubsetMaterialName(int subsetID)
{
	FBXSubsets* subset = mSubsets[subsetID];
	
	if (subset == 0) return "";
	
	return subset->mMaterialName;
}
//===============================================================================================================================
//vector<ZShadeSandboxMesh::FBXTriangle> FBXLoader::SubsetTriangles(int subsetID)
//{
//	FBXSubsets* subset = mSubsets[subsetID];
//	
//	vector<ZShadeSandboxMesh::FBXTriangle> emptyContainer;
//	if (subset == 0) return emptyContainer;
//	
//	return subset->mTriangles;
//}
vector<uint32> FBXLoader::SubsetIndices(int subsetID)
{
	FBXSubsets* subset = mSubsets[subsetID];
	
	vector<uint32> emptyContainer;
	if (subset == 0) return emptyContainer;
	
	return subset->mIndices;
}
//===============================================================================================================================
vector<uint32> FBXLoader::SubsetJoints(int subsetID)
{
	FBXSubsets* subset = mSubsets[subsetID];
	
	vector<uint32> emptyContainer;
	if (subset == 0) return emptyContainer;
	
	return subset->mJoints;
}
//===============================================================================================================================
vector<ZShadeSandboxMesh::VertexNormalTexBlend> FBXLoader::SubsetVertices(int subsetID)
{
	FBXSubsets* subset = mSubsets[subsetID];
	
	vector<ZShadeSandboxMesh::VertexNormalTexBlend> emptyContainer;
	if (subset == 0) return emptyContainer;
	
	return subset->mVertices;
}
//===============================================================================================================================
ZShadeSandboxLighting::ShaderMaterial* FBXLoader::SubsetMaterial(int subsetID, int materialID)
{
	FBXSubsets* subset = mSubsets[subsetID];
	
	if (subset == 0) return 0;
	
	return subset->mMaterials[materialID];
}
//===============================================================================================================================
void FBXLoader::CleanupFbxManager()
{
	m_pFbxScene->Destroy();
	m_pFBXManager->Destroy();

	mSkeleton.joints.clear();

	for (int subsetID = 0; subsetID < mSubsets.size(); subsetID++)
	{
		//mSubsets[subsetID]->mTriangles.clear();
		
		mSubsets[subsetID]->mJoints.clear();
		mSubsets[subsetID]->mIndices.clear();
		mSubsets[subsetID]->mVertices.clear();
		
		for (auto iter = mSubsets[subsetID]->mMaterials.begin(); iter != mSubsets[subsetID]->mMaterials.end(); iter++)
		{
			delete iter->second;
		}

		mSubsets[subsetID]->mMaterials.clear();
	}
}
//===============================================================================================================================
bool FBXLoader::Load(const char* filename)
{
	try
	{
		//
		// Initialize FBX
		//
		
		outFile.open("LoadingFBXMesh.txt", ios::out);

		outVertexFile.open("FBXVertices.txt", ios::out);
		outIndiceFile.open("FBXIndices.txt", ios::out);
		
		mInputFilePath = string(filename);
		
		if (m_pFBXManager == nullptr)
		{
			m_pFBXManager = FbxManager::Create();
			
			FbxIOSettings* pIOSettings = FbxIOSettings::Create(m_pFBXManager, IOSROOT);
			
			m_pFBXManager->SetIOSettings(pIOSettings);
		}
		
		//
		// Load the scene for the model
		//
		
		LoadScene();
		
		//
		// Load the mesh and skeleton
		//
		
		// Need to write a profiler system that does time performance calculation
		
		LARGE_INTEGER start;
		LARGE_INTEGER end;
		
		QueryPerformanceCounter(&start);
		LoadSkeletonHierarchy(m_pFbxScene->GetRootNode());
		if(mSkeleton.joints.empty())
		{
			bHasAnimation = false;
		}
		
		outFile << "\n\n\n\nLoading FBX Model: " <<  filename << "\n";
		QueryPerformanceCounter(&end);
		float finalTime = ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart));
		string finalTimeStr = ZShadeSandboxGlobal::Convert::ConvertToString<float>(finalTime);
		outFile << "Processing Skeleton Hierarchy: " << finalTimeStr << "s\n";
		
		QueryPerformanceCounter(&start);
		LoadGeometry(m_pFbxScene->GetRootNode());
		QueryPerformanceCounter(&end);
		finalTime = ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart));
		finalTimeStr = ZShadeSandboxGlobal::Convert::ConvertToString<float>(finalTime);
		outFile << "Processing Geometry: " << finalTimeStr << "s\n";
		
		/*QueryPerformanceCounter(&start);
		Optimize();
		QueryPerformanceCounter(&end);
		finalTime = ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart));
		finalTimeStr = ZShadeSandboxGlobal::Convert::ConvertToString<float>(finalTime);
		outFile << "Optimization: " << finalTimeStr << "s\n";*/
		
		//CleanupFbxManager();
		outFile << "\n\nFBX Loading Done!\n";
		
		outFile.close();
		outVertexFile.close();
		outIndiceFile.close();
	} catch (std::exception e)
	{
		outFile << "Error loading FBX Model" << e.what() << "!\n";
		outFile.close();
	}
	
	return true;
}
//===============================================================================================================================
bool FBXLoader::LoadScene()
{
	LARGE_INTEGER start;
	LARGE_INTEGER end;
	int fileMinor, fileRevision;
	int sdkMajor, sdkMinor, sdkRevision;
	int fileFormat;
	
	QueryPerformanceCounter(&start);
	{
		FbxString filePath = FbxGetApplicationDirectory();
		m_pFBXManager->LoadPluginsDirectory(filePath.Buffer());
		
		FbxManager::GetFileFormatVersion(sdkMajor, sdkMinor, sdkRevision);
		FbxImporter* pImporter = FbxImporter::Create(m_pFBXManager, "");
		m_pFbxScene = FbxScene::Create(m_pFBXManager, "");
		
		if(!m_pFBXManager->GetIOPluginRegistry()->DetectReaderFileFormat(mInputFilePath.c_str(), fileFormat))
		{
			//Unrecognizable file format. Try to fall back on FbxImorter::eFBX_BINARY
			fileFormat = m_pFBXManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
		}
		
		bool bSuccess = pImporter->Initialize(mInputFilePath.c_str(), fileFormat, m_pFBXManager->GetIOSettings());
		
		pImporter->GetFileVersion(fileMinor, fileMinor, fileRevision);
		
		if(!bSuccess)
		{
			printf( "ERROR %s : %d FbxImporter Initialize failed!\n", __FILE__, __LINE__ );
			return false;
		}
		
		bSuccess = pImporter->Import(m_pFbxScene);
		
		if (!bSuccess)
		{
			return false;
		}
		
		pImporter->Destroy();

		// Very Important!! Triangulate all meshes in the scene
		// DirectX must have this to render the mesh properly
		FbxGeometryConverter clsConv(m_pFBXManager);
		bool triangulate = clsConv.Triangulate(m_pFbxScene, true);
	}
	QueryPerformanceCounter(&end);
	
	float finalTime = ((end.QuadPart - start.QuadPart) / static_cast<float>(mCPUFreq.QuadPart));
	string finalTimeStr = ZShadeSandboxGlobal::Convert::ConvertToString<float>(finalTime);
	outFile << "Loading FBX File: " << finalTimeStr << "s\n";
}
//===============================================================================================================================
void FBXLoader::Optimize()
{
	/*for (int subsetID = 0; subsetID < mSubsets.size(); subsetID++)
	{
		std::vector<ZShadeSandboxMesh::VertexNormalTexBlend> uniqueVertices;

		for (uint32 i = 0; i < mSubsets[subsetID]->mTriangles.size(); ++i)
		{
			for (uint32 j = 0; j < 3; ++j)
			{
				// If the current vertex is not in the unique list then add it
				if (FindVertex(mSubsets[subsetID]->mVertices[i * 3 + j], uniqueVertices) == -1)
				{
					uniqueVertices.push_back(mSubsets[subsetID]->mVertices[i * 3 + j]);
				}
			}
		}

		// Regenerate the index buffer
		for (uint32 i = 0; i < mSubsets[subsetID]->mTriangles.size(); ++i)
		{
			for (uint32 j = 0; j < 3; ++j)
			{
				mSubsets[subsetID]->mTriangles[i].indices[j] = FindVertex(mSubsets[subsetID]->mVertices[i * 3 + j], uniqueVertices);
			}
		}

		mSubsets[subsetID]->mVertices.clear();
		mSubsets[subsetID]->mVertices = uniqueVertices;
		uniqueVertices.clear();

		// Sort the triangles by materials
		std::sort(mSubsets[subsetID]->mTriangles.begin(), mSubsets[subsetID]->mTriangles.end());
	}*/
}
//===============================================================================================================================
int FBXLoader::FindVertex(const ZShadeSandboxMesh::VertexNormalTexBlend& inTargetVertex, const std::vector<ZShadeSandboxMesh::VertexNormalTexBlend>& uniqueVertices)
{
	for (uint32 i = 0; i < uniqueVertices.size(); ++i)
	{
		if (inTargetVertex == uniqueVertices[i])
		{
			return i;
		}
	}
	
	return -1;
}
//===============================================================================================================================
void FBXLoader::LoadMesh(FbxNode* node)
{
	FBXSubsets* newSubset = new FBXSubsets();
	
	FbxMesh* mesh = node->GetMesh();
	
	newSubset->mPolygonCount = mesh->GetPolygonCount();
	
	int vertexCounter = 0;
	
	// http://stackoverflow.com/questions/30170521/how-to-read-in-fbx-2014-indices-properly-for-directx

	/*newSubset->mTriangles.reserve(newSubset->mPolygonCount);
	
	for (uint32 i = 0; i < newSubset->mPolygonCount; ++i)
	{
		XMFLOAT3 normal[3];
		XMFLOAT3 tangent[3];
		XMFLOAT3 binormal[3];
		XMFLOAT2 uv[3][2];
		
		ZShadeSandboxMesh::FBXTriangle triangle;
		
		newSubset->mTriangles.push_back(triangle);
		
		for (uint32 j = 0; j < 3; ++j)
		{
			int controlPointIndex = mesh->GetPolygonVertex(i, j);
			ZShadeSandboxMesh::PhysicalPoint* ctlPoint = mControlPoints[controlPointIndex];
			
			LoadVertexNormal(mesh, controlPointIndex, vertexCounter, 0, normal[j]);
			
			// Only have diffuse texture
			for (int k = 0; k < 1; ++k)
			{
				LoadVertexTexture(mesh, controlPointIndex, mesh->GetTextureUVIndex(i, j), k, uv[j][k]);
			}
			
			//LoadVertexTangent(mesh, controlPointIndex, vertexCounter, 0, tangent[j]);

			ZShadeSandboxMesh::VertexNormalTexBlend temp;
			temp.position = ctlPoint->position;
			temp.normal = normal[j];
			//temp.tangent = tangent[j];
			temp.texture = uv[j][0];
			
			// Copy the blending from each control point
			for (uint32 i = 0; i < ctlPoint->blendingInfo.size(); ++i)
			{
				ZShadeSandboxMesh::VertexBlendingInfo blendingInfo;
				
				blendingInfo.blendingIndex = ctlPoint->blendingInfo[i].blendingIndex;
				blendingInfo.blendingWeight = ctlPoint->blendingInfo[i].blendingWeight;
				
				temp.vertexBlendingInfos.push_back(blendingInfo);
			}
			
			// Sort blending info to remove duplicate vertices
			temp.SortBlendingInfoByWeight();
			
			newSubset->mVertices.push_back(temp);
			newSubset->mTriangles.back().indices.push_back(vertexCounter);
			
			++vertexCounter;
		}
	}*/
	
	/*int* indices = mesh->GetPolygonVertices();
	for (int ind = 0; ind < mesh->GetPolygonVertexCount(); ind++)
	{
		newSubset->mIndices.push_back(indices[ind]);

		string indice = ZShadeSandboxGlobal::Convert::ConvertToString<int>(indices[ind]);
		outIndiceFile << "indice: " << indice << "\n";
	}*/

	for (uint32 polygonID = 0; polygonID < newSubset->mPolygonCount; ++polygonID)
	{
		int polyVertCount = mesh->GetPolygonSize(polygonID);
		
		for (uint32 polygonVertexID = 0; polygonVertexID < polyVertCount; ++polygonVertexID)
		{
			ZShadeSandboxMesh::VertexNormalTexBlend temp;
			
			// Initialize the vertex data
			temp.position = XMFLOAT3(0, 0, 0);
			temp.normal = XMFLOAT3(0, 0, 0);
			temp.texture = XMFLOAT2(0, 0);
			temp.tangent = XMFLOAT3(0, 0, 0);
			
			int controlPointIndex = mesh->GetPolygonVertex(polygonID, polygonVertexID);
			
			ZShadeSandboxMesh::PhysicalPoint* ctlPoint = mControlPoints[controlPointIndex];
			
			//
			// Load vertex position
			//
			
			temp.position = ctlPoint->position;
			
			//
			// Load vertex normal
			//
			
			int normElementCount = mesh->GetElementNormalCount();
			
			for (int normElement = 0; normElement < normElementCount; normElement++)
			{
				XMFLOAT3 normal;
				
				if (LoadVertexNormal(mesh, controlPointIndex, vertexCounter, normElement, normal))
				{
					temp.normal = normal;
					
					break;
				}
			}
			
			//
			// Load vertex UV
			//
			
			int uvElementCount = mesh->GetElementUVCount();
			
			for (int uvElement = 0; uvElement < uvElementCount; uvElement++)
			{
				XMFLOAT2 uv;
				
				if (LoadVertexTexture(mesh, controlPointIndex, mesh->GetTextureUVIndex(polygonID, polygonVertexID), uvElement, uv))
				{
					temp.texture = uv;
					
					break;
				}
			}
			
			//
			// Load vertex tangent
			//
			
			int tangentElementCount = mesh->GetElementTangentCount();
			
			for (int tangentElement = 0; tangentElement < tangentElementCount; tangentElement++)
			{
				XMFLOAT3 tangent;
				
				if (LoadVertexTangent(mesh, controlPointIndex, vertexCounter, tangentElement, tangent))
				{
					temp.tangent = tangent;
					
					break;
				}
			}
			
			//
			// Load vertex blending information for skinning
			//
			
			// Copy the blending from each control point
			for (uint32 i = 0; i < ctlPoint->blendingInfo.size(); ++i)
			{
				ZShadeSandboxMesh::VertexBlendingInfo blendingInfo;
				
				blendingInfo.blendingIndex = ctlPoint->blendingInfo[i].blendingIndex;
				blendingInfo.blendingWeight = ctlPoint->blendingInfo[i].blendingWeight;
				
				temp.vertexBlendingInfos.push_back(blendingInfo);
			}
			
			// Sort blending info to remove duplicate vertices
			temp.SortBlendingInfoByWeight();
			
			//
			// Make sure the vertices are unique and get the index
			//
			
			vector<ZShadeSandboxMesh::VertexNormalTexBlend>& uniqueVerts = newSubset->mVertices;
			
			size_t size = uniqueVerts.size();
			uint32 indice;
			
			for (indice = 0; indice < size; indice++)
			{
				if (temp.EqualsPosNormTex(uniqueVerts[indice]))
				{
					break;
				}
			}
			
			if (indice == size)
			{
				uniqueVerts.push_back(temp);
				string pos = ZShadeSandboxGlobal::Convert::ConvertFloat3ToString(temp.position);
				outVertexFile << "vertex: " << pos << "\n";
			}
			
			newSubset->mIndices.push_back(indice);
			string indice_str = ZShadeSandboxGlobal::Convert::ConvertToString<uint32>(indice);
			outIndiceFile << "indice: " << indice_str << "\n";
			
			++vertexCounter;
		}
	}

	// Now mControlPoints has served its purpose so we can free its memory
	for(auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr)
	{
		delete itr->second;
	}
	
	mControlPoints.clear();
	
	// Adding a new subset to the mesh
	mSubsets.push_back(newSubset);
}
//===============================================================================================================================
bool FBXLoader::LoadVertexTexture(FbxMesh* mesh, int inCtrlPointIndex, int inTextureIndex, int inTextureLayer, XMFLOAT2& outTexture)
{
	if (inTextureLayer >= 2 || mesh->GetElementUVCount() <= inTextureLayer)
	{
		throw std::exception("Invalid UV Layer Number");
	}
	
	int directIndex = -1;
	
	FbxGeometryElementUV* vertexUV = mesh->GetElementUV(inTextureLayer);
	
	switch (vertexUV->GetMappingMode())
	{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (vertexUV->GetReferenceMode())
			{
				case FbxGeometryElement::eDirect:
				{
					directIndex = inCtrlPointIndex;
					//outTexture.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
					//outTexture.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					directIndex = vertexUV->GetIndexArray().GetAt(inCtrlPointIndex);
					//int index = vertexUV->GetIndexArray().GetAt(inCtrlPointIndex);
					//outTexture.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
					//outTexture.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
				}
				break;
				default: throw std::exception("Invalid Reference");
			}
		}
		break;
		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (vertexUV->GetReferenceMode())
			{
				case FbxGeometryElement::eDirect:
				case FbxGeometryElement::eIndexToDirect:
				{
					directIndex = inTextureIndex;
					//outTexture.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureIndex).mData[0]);
					//outTexture.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureIndex).mData[1]);
				}
				break;
				default: throw std::exception("Invalid Reference");
			}
		}
		break;
	}
	
	if (directIndex != -1)
	{
		FbxVector2 uv = vertexUV->GetDirectArray().GetAt(directIndex);
		
		outTexture = XMFLOAT2((float)uv.mData[0], (float)uv.mData[1]);
		
		return true;
	}
	
	return false;
}
//===============================================================================================================================
bool FBXLoader::LoadVertexNormal(FbxMesh* mesh, int inCtrlPointIndex, int inVertexCounter, int normalElement, XMFLOAT3& outNormal)
{
	if (mesh->GetElementNormalCount() < 1)
	{
		throw std::exception("Invalid Normal Number");
	}
	
	int directIndex = -1;
	
	FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(normalElement);
	
	switch (vertexNormal->GetMappingMode())
	{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (vertexNormal->GetReferenceMode())
			{
				case FbxGeometryElement::eDirect:
				{
					directIndex = inCtrlPointIndex;
					//outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
					//outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
					//outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					directIndex = vertexNormal->GetIndexArray().GetAt(inCtrlPointIndex);
					//int index = vertexNormal->GetIndexArray().GetAt(inCtrlPointIndex);
					//outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
					//outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
					//outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default: throw std::exception("Invalid Reference");
			}
			break;
		}
		break;
		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (vertexNormal->GetReferenceMode())
			{
				case FbxGeometryElement::eDirect:
				{
					directIndex = inVertexCounter;
					//outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
					//outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
					//outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					directIndex = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
					//int index = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
					//outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
					//outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
					//outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default: throw std::exception("Invalid Reference");
			}
		}
		break;
	}
	
	if (directIndex != -1)
	{
		FbxVector4 norm = vertexNormal->GetDirectArray().GetAt(directIndex);
		
		outNormal = XMFLOAT3((float)norm.mData[0], (float)norm.mData[1], (float)norm.mData[2]);
		
		return true;
	}
	
	return false;
}
//===============================================================================================================================
bool FBXLoader::LoadVertexBinormal(FbxMesh* mesh, int inCtrlPointIndex, int inVertexCounter, int binormalElement, XMFLOAT3& outBinormal)
{
	if (mesh->GetElementBinormalCount() < 1)
	{
		throw std::exception("Invalid Binormal Number");
	}
	
	int directIndex = -1;
	
	FbxGeometryElementBinormal* vertexBinormal = mesh->GetElementBinormal(binormalElement);
	
	switch (vertexBinormal->GetMappingMode())
	{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (vertexBinormal->GetReferenceMode())
			{
				case FbxGeometryElement::eDirect:
				{
					directIndex = inCtrlPointIndex;
					//outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
					//outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
					//outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					directIndex = vertexBinormal->GetIndexArray().GetAt(inCtrlPointIndex);
					//int index = vertexBinormal->GetIndexArray().GetAt(inCtrlPointIndex);
					//outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
					//outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
					//outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default: throw std::exception("Invalid Reference");
			}
		}
		break;
		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (vertexBinormal->GetReferenceMode())
			{
				case FbxGeometryElement::eDirect:
				{
					directIndex = inVertexCounter;
					//outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
					//outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
					//outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					directIndex = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
					//int index = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
					//outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
					//outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
					//outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default: throw std::exception("Invalid Reference");
			}
		}
		break;
	}
	
	if (directIndex != -1)
	{
		FbxVector4 binorm = vertexBinormal->GetDirectArray().GetAt(directIndex);
		
		outBinormal = XMFLOAT3((float)binorm.mData[0], (float)binorm.mData[1], (float)binorm.mData[2]);
		
		return true;
	}
	
	return false;
}
//===============================================================================================================================
bool FBXLoader::LoadVertexTangent(FbxMesh* mesh, int inCtrlPointIndex, int inVertexCounter, int tangentElement, XMFLOAT3& outTangent)
{
	if (mesh->GetElementTangentCount() < 1)
	{
		throw std::exception("Invalid Tangent Number");
	}
	
	int directIndex = -1;
	
	FbxGeometryElementTangent* vertexTangent = mesh->GetElementTangent(tangentElement);
	
	switch (vertexTangent->GetMappingMode())
	{
		case FbxGeometryElement::eByControlPoint:
		{
			switch (vertexTangent->GetReferenceMode())
			{
				case FbxGeometryElement::eDirect:
				{
					directIndex = inCtrlPointIndex;
					//outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
					//outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
					//outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					directIndex = vertexTangent->GetIndexArray().GetAt(inCtrlPointIndex);
					//int index = vertexTangent->GetIndexArray().GetAt(inCtrlPointIndex);
					//outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
					//outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
					//outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default: throw std::exception("Invalid Reference");
			}
		}
		break;
		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (vertexTangent->GetReferenceMode())
			{
				case FbxGeometryElement::eDirect:
				{
					directIndex = inVertexCounter;
					//outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[0]);
					//outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[1]);
					//outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[2]);
				}
				break;
				case FbxGeometryElement::eIndexToDirect:
				{
					directIndex = vertexTangent->GetIndexArray().GetAt(inVertexCounter);
					//int index = vertexTangent->GetIndexArray().GetAt(inVertexCounter);
					//outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
					//outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
					//outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
				}
				break;
				default: throw std::exception("Invalid Reference");
			}
		}
		break;
	}
	
	if (directIndex != -1)
	{
		FbxVector4 tan = vertexTangent->GetDirectArray().GetAt(directIndex);
		
		outTangent = XMFLOAT3((float)tan.mData[0], (float)tan.mData[1], (float)tan.mData[2]);
		
		return true;
	}
	
	return false;
}
//===============================================================================================================================
uint32 FBXLoader::FindJointIndexUsingName(string& name)
{
	for (uint32 i = 0; i < mSkeleton.joints.size(); ++i)
	{
		if (mSkeleton.joints[i].name == name)
		{
			return i;
		}
	}
	
	throw std::exception("Skeleton information in FBX file is corrupted.");
}
//===============================================================================================================================
void FBXLoader::LoadGeometry(FbxNode* node)
{
	if (node->GetNodeAttribute())
	{
		switch (node->GetNodeAttribute()->GetAttributeType())
		{
			case FbxNodeAttribute::eMesh:
			{
				LoadControlPoints(node);
				
				if (bHasAnimation)
				{
					LoadJointsAndAnimation(node);
				}
				
				LoadMesh(node);
				AssociateMaterialToMesh(node);
				LoadMaterials(node);
				
				// ready for next mesh subset
				iCurrentSubset++;
			}
			break;
		}
	}
	
	for (int i = 0; i < node->GetChildCount(); ++i)
	{
		LoadGeometry(node->GetChild(i));
	}
}
//===============================================================================================================================
void FBXLoader::LoadControlPoints(FbxNode* node)
{
	FbxMesh* mesh = node->GetMesh();
	uint32 controlPointCount = mesh->GetControlPointsCount();
	
	for (uint32 i = 0; i < controlPointCount; ++i)
	{
		ZShadeSandboxMesh::PhysicalPoint* currCtrlPoint = new ZShadeSandboxMesh::PhysicalPoint();
		
		XMFLOAT3 position;
		position.x = static_cast<float>(mesh->GetControlPointAt(i).mData[0]);
		position.y = static_cast<float>(mesh->GetControlPointAt(i).mData[1]);
		position.z = static_cast<float>(mesh->GetControlPointAt(i).mData[2]);
		currCtrlPoint->position = position;
		
		mControlPoints[i] = currCtrlPoint;
	}
}
//===============================================================================================================================
void FBXLoader::LoadSkeletonHierarchy(FbxNode* inRootNode)
{
	for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
	{
		FbxNode* node = inRootNode->GetChild(childIndex);
		
		LoadSkeletonHierarchy(node, 0, 0, -1);
	}
}
//===============================================================================================================================
void FBXLoader::LoadSkeletonHierarchy(FbxNode* inNode, int inDepth, int myIndex, int parentIndex)
{
	if (inNode->GetNodeAttribute() &&
	    inNode->GetNodeAttribute()->GetAttributeType() &&
		inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		ZShadeSandboxMesh::FBXJoint currJoint;
		currJoint.parentIndex = parentIndex;
		currJoint.name = inNode->GetName();
		mSkeleton.joints.push_back(currJoint);
	}
	
	for (int i = 0; i < inNode->GetChildCount(); i++)
	{
		LoadSkeletonHierarchy(inNode->GetChild(i), inDepth + 1, mSkeleton.joints.size(), myIndex);
	}
}
//===============================================================================================================================
void FBXLoader::LoadJointsAndAnimation(FbxNode* inNode)
{
	// This is how each subset gets its bone/joint for animation
	FBXSubsets* subset = mSubsets[iCurrentSubset];
	
	FbxMesh* mesh = inNode->GetMesh();
	
	uint32 numOfDeformers = mesh->GetDeformerCount();
	
	FbxAMatrix geomTrans = ZShadeSandboxMesh::FBXHelper::GetGeometryTransformation(inNode);
	
	// A deformer contains clusters.
	// A cluster contains a link, which is a joint.
	// Normally, There is only one deformer in a mesh but Maya has many types.
	for (uint32 deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
	{
		// Lets see if this deformer is a skin
		FbxSkin* skin = reinterpret_cast<FbxSkin*>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		
		if (!skin) continue;
		
		uint32 numOfClusters = skin->GetClusterCount();
		
		for (uint32 clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
		{
			FbxCluster* cluster = skin->GetCluster(clusterIndex);
			
			string jointName = cluster->GetLink()->GetName();
			uint32 jointIndex = FindJointIndexUsingName(jointName);
			
			subset->mJoints.push_back(jointIndex);
			
			FbxAMatrix transform;
			FbxAMatrix transformLink;
			FbxAMatrix globalBindposeInverse;
			
			// The transformation of the mesh at binding time
			cluster->GetTransformMatrix(transform);
			
			// The transformation of the cluster (joint) at binding time from joint space to world space
			cluster->GetTransformLinkMatrix(transformLink);
			
			globalBindposeInverse = transformLink.Inverse() * transform * geomTrans;
			
			
			// Update skeletal information
			mSkeleton.joints[jointIndex].globalBindposeInverse = globalBindposeInverse;
			mSkeleton.joints[jointIndex].node = cluster->GetLink();
			
			// Associate each joint with the control points it affects
			uint32 numOfIndices = cluster->GetControlPointIndicesCount();
			
			for (uint32 i = 0; i < numOfIndices; ++i)
			{
				ZShadeSandboxMesh::BlendingIndexWeightPair currBlendingIndexWeightPair;
				currBlendingIndexWeightPair.blendingIndex = jointIndex;
				currBlendingIndexWeightPair.blendingWeight = cluster->GetControlPointWeights()[i];
				mControlPoints[cluster->GetControlPointIndices()[i]]->blendingInfo.push_back(currBlendingIndexWeightPair);
			}
			
			// Animation information
			FbxAnimStack* animStack = m_pFbxScene->GetSrcObject<FbxAnimStack>(0);
			FbxString animStackName = animStack->GetName();
			mAnimationName = animStackName.Buffer();
			FbxTakeInfo* takeInfo = m_pFbxScene->GetTakeInfo(animStackName);
			FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
			mAnimationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
			FBXKeyframe** anim = &mSkeleton.joints[jointIndex].animation;
			
			for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i)
			{
				FbxTime time;
				time.SetFrame(i, FbxTime::eFrames24);
				*anim = new FBXKeyframe();
				(*anim)->frameNum = i;
				FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(time) * geomTrans;
				(*anim)->globalTransform = currentTransformOffset.Inverse() * cluster->GetLink()->EvaluateGlobalTransform(time);
				anim = &((*anim)->next);
			}
		}
	}
	
	// Some control points have less than 4 joints
	// For a normal renderer, there are usually 4 joints
	ZShadeSandboxMesh::BlendingIndexWeightPair currBlendingIndexWeightPair;
	currBlendingIndexWeightPair.blendingIndex = 0;
	currBlendingIndexWeightPair.blendingWeight = 0;
	for (auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr)
	{
		for (unsigned int i = itr->second->blendingInfo.size(); i <= 4; ++i)
		{
			itr->second->blendingInfo.push_back(currBlendingIndexWeightPair);
		}
	}
}
//===============================================================================================================================
void FBXLoader::AssociateMaterialToMesh(FbxNode* inNode)
{
	FbxLayerElementArrayTemplate<int>* materialIndices;
	FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;
	FbxMesh* mesh = inNode->GetMesh();
	
	if (mesh->GetElementMaterial())
	{
		materialIndices = &(mesh->GetElementMaterial()->GetIndexArray());
		materialMappingMode = mesh->GetElementMaterial()->GetMappingMode();
		
		FBXSubsets* subset = mSubsets[iCurrentSubset];
		
		if (materialIndices)
		{
			switch (materialMappingMode)
			{
				case FbxGeometryElement::eByPolygon:
				{
					if (materialIndices->GetCount() == subset->mPolygonCount)
					{
						for (uint32 i = 0; i < subset->mPolygonCount; ++i)
						{
							uint32 materialIndex = materialIndices->GetAt(i);
							//subset->mTriangles[i].materialIndex = materialIndex;
							subset->mMaterialIndex = materialIndex;
						}
					}
				}
				break;
				case FbxGeometryElement::eAllSame:
				{
					uint32 materialIndex = materialIndices->GetAt(0);
					for (uint32 i = 0; i < subset->mPolygonCount; ++i)
					{
						//subset->mTriangles[i].materialIndex = materialIndex;
						subset->mMaterialIndex = materialIndex;
					}
				}
				break;
				default: throw std::exception("Invalid mapping mode for material\n");
			}
		}
	}
}
//===============================================================================================================================
void FBXLoader::LoadMaterials(FbxNode* inNode)
{
	uint32 materialCount = inNode->GetMaterialCount();
	
	for (uint32 i = 0; i < materialCount; ++i)
	{
		for (uint32 subsetID = 0; subsetID < mSubsets.size(); subsetID++)
		{
			if (mSubsets[subsetID]->mMaterialIndex == i)
			{
				FbxSurfaceMaterial* surfaceMaterial = inNode->GetMaterial(i);
				LoadMaterialAttribute(surfaceMaterial, i, subsetID);
				SetMaterialName(mSubsets[subsetID]->mMaterials[i]);
				LoadMaterialTexture(surfaceMaterial, mSubsets[subsetID]->mMaterials[i]);
				
				mSubsets[subsetID]->mMaterialName = mSubsets[subsetID]->mMaterials[i]->sMaterialName;
				
				// Go to the next material
				break;
			}
		}
	}
}
//===============================================================================================================================
void FBXLoader::LoadMaterialAttribute(FbxSurfaceMaterial* inMaterial, uint32 inMaterialIndex, uint32 subsetID)
{
	FbxDouble3 double3;
	FbxDouble double1;
	
	FBXSubsets* subset = mSubsets[subsetID];
	
	if (inMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		ZShadeSandboxLighting::ShaderMaterial* material = new ZShadeSandboxLighting::ShaderMaterial();
		
		// Ambient Color
		double3 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->Ambient;
		material->vAmbientColor.x = static_cast<float>(double3.mData[0]);
		material->vAmbientColor.y = static_cast<float>(double3.mData[1]);
		material->vAmbientColor.z = static_cast<float>(double3.mData[2]);
		
		// Diffuse Color
		double3 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->Diffuse;
		material->vDiffuseColor.x = static_cast<float>(double3.mData[0]);
		material->vDiffuseColor.y = static_cast<float>(double3.mData[1]);
		material->vDiffuseColor.z = static_cast<float>(double3.mData[2]);
		
		// Specular Color
		double3 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->Specular;
		material->vSpecularColor.x = static_cast<float>(double3.mData[0]);
		material->vSpecularColor.y = static_cast<float>(double3.mData[1]);
		material->vSpecularColor.z = static_cast<float>(double3.mData[2]);
		
		// Emissive Color
		double3 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->Emissive;
		material->vEmissiveColor.x = static_cast<float>(double3.mData[0]);
		material->vEmissiveColor.y = static_cast<float>(double3.mData[1]);
		material->vEmissiveColor.z = static_cast<float>(double3.mData[2]);
		
		// Reflection
		double3 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->Reflection;
		material->vReflection.x = static_cast<float>(double3.mData[0]);
		material->vReflection.y = static_cast<float>(double3.mData[1]);
		material->vReflection.z = static_cast<float>(double3.mData[2]);
		
		// Alpha transparency factor
		double1 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->TransparencyFactor;
		material->fAlpha = double1;
		
		// Material Shininess
		double1 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->Shininess;
		material->fSpecularIntensity = double1;
		
		// Specular Power
		double1 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->SpecularFactor;
		material->fSpecularPower = double1;
		
		// Material reflectivity factor
		double1 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->ReflectionFactor;
		material->fReflectivity = double1;
		
		subset->mMaterials[inMaterialIndex] = material;
	}
	else if (inMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		ZShadeSandboxLighting::ShaderMaterial* material = new ZShadeSandboxLighting::ShaderMaterial();
		
		// Ambient Color
		double3 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->Ambient;
		material->vAmbientColor.x = static_cast<float>(double3.mData[0]);
		material->vAmbientColor.y = static_cast<float>(double3.mData[1]);
		material->vAmbientColor.z = static_cast<float>(double3.mData[2]);
		
		// Diffuse Color
		double3 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->Diffuse;
		material->vDiffuseColor.x = static_cast<float>(double3.mData[0]);
		material->vDiffuseColor.y = static_cast<float>(double3.mData[1]);
		material->vDiffuseColor.z = static_cast<float>(double3.mData[2]);
		
		// Emissive Color
		double3 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->Emissive;
		material->vEmissiveColor.x = static_cast<float>(double3.mData[0]);
		material->vEmissiveColor.y = static_cast<float>(double3.mData[1]);
		material->vEmissiveColor.z = static_cast<float>(double3.mData[2]);
		
		// Alpha transparency factor
		double1 = reinterpret_cast<FbxSurfacePhong*>(inMaterial)->TransparencyFactor;
		material->fAlpha = double1;
		
		subset->mMaterials[inMaterialIndex] = material;
	}
}
//===============================================================================================================================
void FBXLoader::SetMaterialName(ZShadeSandboxLighting::ShaderMaterial*& ioMaterial)
{
	string materialName = "Material";
	
	materialName += ZShadeSandboxGlobal::Convert::ConvertToString<int>(iFbxMaterialCounter + 1);
	
	ioMaterial->sMaterialName = materialName;
	
	// Get ready for next material
	iFbxMaterialCounter++;
}
//===============================================================================================================================
void FBXLoader::LoadMaterialTexture(FbxSurfaceMaterial* inMaterial, ZShadeSandboxLighting::ShaderMaterial*& ioMaterial)
{
	uint32 textureIndex = 0;
	FbxProperty property;
	
	FBXSDK_FOR_EACH_TEXTURE(textureIndex)
	{
		property = inMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[textureIndex]);
		if (property.IsValid())
		{
			uint32 textureCount = property.GetSrcObjectCount<FbxTexture>();
			for (uint32 i = 0; i < textureCount; ++i)
			{
				FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);
				if(layeredTexture)
				{
					throw std::exception("Layered Texture is currently unsupported\n");
				}
				else
				{
					FbxTexture* texture = property.GetSrcObject<FbxTexture>(i);
					if (texture)
					{
						std::string textureType = property.GetNameAsCStr();
						FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);

						if (fileTexture)
						{
							string str_filename(fileTexture->GetFileName());
							std::size_t index = str_filename.find_last_of('/\\');
							if (index != string::npos)
							{
								BetterString str(fileTexture->GetFileName());
								int dot_index = str.get_index('.');
								BetterString pathName = str.substring(index + 1, dot_index);
								BetterString ext = str.substring(dot_index + 1);
								BetterString filename = pathName + "." + ext;
								str_filename = filename.toString();
							}

							if (textureType == "DiffuseColor")
							{
								ioMaterial->sDiffuseTextureName = str_filename;
								ioMaterial->bHasDiffuseTexture = true;
							}
							else if (textureType == "SpecularColor")
							{
								ioMaterial->sSpecularTextureName = str_filename;
								ioMaterial->bHasSpecularTexture = true;
							}
							else if (textureType == "Bump")
							{
								ioMaterial->sNormalMapTextureName = str_filename;
								ioMaterial->bHasNormalMapTexture = true;
							}
						}
					}
				}
			}
		}
	}
	
	ioMaterial->SetD3D(m_pD3DSystem);
	
	bool addToMM = false;
	
	if (ioMaterial->bHasDiffuseTexture)
	{
		ZShadeSandboxLighting::ShaderMaterial* material;
		if (material = MaterialManager::Instance()->GetMaterial(ioMaterial->sMaterialName))
		{
			// The texture has already been loaded
			ioMaterial->SetMaterialDiffuseTexture(material->DiffuseTexture());
		}
		else
		{
			// Need to load new texture
			ioMaterial->AddDiffuseTexture(m_pGD3D->m_textures_path, ioMaterial->sDiffuseTextureName);
			
			addToMM = true;
		}
	}
	
	if (ioMaterial->bHasSpecularTexture)
	{
		ZShadeSandboxLighting::ShaderMaterial* material;
		if (material = MaterialManager::Instance()->GetMaterial(ioMaterial->sMaterialName))
		{
			// The texture has already been loaded
			ioMaterial->SetMaterialSpecularTexture(material->SpecularTexture());
		}
		else
		{
			// Need to load new texture
			ioMaterial->AddDiffuseTexture(m_pGD3D->m_textures_path, ioMaterial->sSpecularTextureName);
			
			addToMM = true;
		}
	}
	
	if (ioMaterial->bHasNormalMapTexture)
	{
		ZShadeSandboxLighting::ShaderMaterial* material;
		if (material = MaterialManager::Instance()->GetMaterial(ioMaterial->sMaterialName))
		{
			// The texture has already been loaded
			ioMaterial->SetMaterialNormalMapTexture(material->NormalMapTexture());
		}
		else
		{
			// Need to load new texture
			ioMaterial->AddDiffuseTexture(m_pGD3D->m_textures_path, ioMaterial->sNormalMapTextureName);
			
			addToMM = true;
		}
	}
	
	if (addToMM)
	{
		MaterialManager::Instance()->Add(ioMaterial);
	}
}
//===============================================================================================================================