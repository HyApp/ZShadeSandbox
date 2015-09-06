//===============================================================================================================================
// MeshAttributes
//
//===============================================================================================================================
// History
//
// -Created on 6/12/2015 by Dustin Watson
//===============================================================================================================================
#ifndef __MESHATTRIBUTES_H
#define __MESHATTRIBUTES_H
//===============================================================================================================================
//===============================================================================================================================

//
// Includes
//

#include "D3D.h"
#include "Vertex.h"
#include "MeshRenderType.h"

//===============================================================================================================================
//===============================================================================================================================
namespace ZShadeSandboxMesh
{
	struct MeshAttributes
	{
		XMFLOAT3											mPosition;
		XMFLOAT3											mScale;
		XMFLOAT3											mRotation;
		int													mVertexCount;
		int													mIndexCount;
		int			 			  							mTriangleCount;
		int													mInstanceCount;
		UINT 					  							mVertexByteWidth;
		UINT												mInstanceByteWidth;
		vector<ZShadeSandboxMesh::VertexPos>				mVerticesPos;
		vector<ZShadeSandboxMesh::VertexColor>				mVerticesColor;
		vector<ZShadeSandboxMesh::VertexTex>				mVerticesTex;
		vector<ZShadeSandboxMesh::VertexNormalTex>			mVerticesNormalTex;
		vector<ZShadeSandboxMesh::VertexNormalTexTan>		mVerticesNormalTexTan;
		vector<ZShadeSandboxMesh::VertexNormalTexTanBiTan>	mVerticesNormalTexTanBi;
		vector<UINT>										mIndices;
		vector<ZShadeSandboxMesh::InstanceMatrix>			mInstanceMatrix;
		ID3D11Buffer*										mVertexBuffer;
		ID3D11Buffer*										mIndexBuffer;
		ID3D11Buffer*										mInstanceBuffer;
		D3D*												mD3D;
		bool												mIsVBDynamic;
		
		
		MeshAttributes();

		void Release();

		void Init();
		
		XMMATRIX WorldXM();
		
		//Returns a billboard rotation to the camera
		XMMATRIX BillboardWorldXM(XMFLOAT3 eye);
		
		void Subdivide(ZShadeSandboxMesh::EVertexType::Type type);

		// Convenient functions to subdivide a custom mesh into smaller triangles
		template<class TVertexType>
		void SubdivideVertex(vector<TVertexType>& verts)
		{
			//       v1
			//       *
			//      / \
			//     /   \
			//  m0*-----*m1
			//   / \   / \
			//  /   \ /   \
			// *-----*-----*
			// v0    m2     v2
			
			// As long as the mesh is dynamic it can subdivide in real time
			
			vector<UINT> ind;
			vector<TVertexType> v;
			GrabSubdividedGeometry(verts, mIndices, v, ind);
			
			mVertexCount = v.size();
			
			mIndices.resize(0);
			verts.resize(0);

			// Reply the mesh with the new subdivision
			
			for (UINT i = 0; i < mVertexCount; i++)
			{
				verts.push_back(v[i]);
			}
			
			ReplyMeshIndices(ind);
		}
		
		template<class TVertexType>
		void GrabSubdividedGeometry
		(	vector<TVertexType> verts
		,	vector<UINT> inds
		,	vector<TVertexType>& OutVerts
		,	vector<UINT>& OutInd
		)
		{
			UINT numTris = mIndexCount / 3;
			
			// Loop through the amount of triangles and subdivide them
			for (UINT i = 0; i < numTris; ++i)
			{
				TVertexType v0 = verts[inds[i * 3 + 0]];
				TVertexType v1 = verts[inds[i * 3 + 1]];
				TVertexType v2 = verts[inds[i * 3 + 2]];
				
				// Create the midpoints of each vertex
				TVertexType m0, m1, m2;
				
				m0.position.x = 0.5f * (v0.position.x + v1.position.x);
				m0.position.y = 0.5f * (v0.position.y + v1.position.y);
				m0.position.z = 0.5f * (v0.position.z + v1.position.z);
				
				m1.position.x = 0.5f * (v1.position.x + v2.position.x);
				m1.position.y = 0.5f * (v1.position.y + v2.position.y);
				m1.position.z = 0.5f * (v1.position.z + v2.position.z);
				
				m2.position.x = 0.5f * (v0.position.x + v2.position.x);
				m2.position.y = 0.5f * (v0.position.y + v2.position.y);
				m2.position.z = 0.5f * (v0.position.z + v2.position.z);
				
				// Add the new geometry
				OutVerts.push_back(v0);
				OutVerts.push_back(v1);
				OutVerts.push_back(v2);
				OutVerts.push_back(m0);
				OutVerts.push_back(m1);
				OutVerts.push_back(m2);
				
				// Link the indices for the new geometry
				OutInd.push_back(i * 6 + 0);
				OutInd.push_back(i * 6 + 3);
				OutInd.push_back(i * 6 + 5);
				
				OutInd.push_back(i * 6 + 3);
				OutInd.push_back(i * 6 + 4);
				OutInd.push_back(i * 6 + 5);
				
				OutInd.push_back(i * 6 + 5);
				OutInd.push_back(i * 6 + 4);
				OutInd.push_back(i * 6 + 2);
				
				OutInd.push_back(i * 6 + 3);
				OutInd.push_back(i * 6 + 1);
				OutInd.push_back(i * 6 + 4);
			}
		}
		
		void ReplyMeshIndices(vector<UINT> ind);
		
		void SetPrimitiveTopology(ZShadeSandboxMesh::ERenderType::Type renderType);
		
		void SetBuffers(bool useIndexBuffer = true);
		void SetBuffersInstanced(bool useIndexBuffer = true);
		void SetBuffers(ZShadeSandboxMesh::ERenderType::Type renderType, bool useIndexBuffer = true);
		void SetBuffersInstanced(ZShadeSandboxMesh::ERenderType::Type renderType, bool useIndexBuffer = true);
		
		void AddInstanceMatrix(
			vector<XMFLOAT3> instancePositions,
			vector<XMFLOAT3> instanceRotations,
			vector<XMFLOAT3> instanceScales
		);
		
		void AddInstanceMatrix(vector<ZShadeSandboxMesh::InstanceMatrix> instMat);

		void BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type type, bool dynamic = true);
		void ReBuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type type);

		template<class TVertexType>
		void BuildVertexBuffer(vector<TVertexType> vec, bool dynamic)
		{
			//
			// Create Vertex Buffer
			//
			
			mIsVBDynamic = dynamic;
			
			mVertexByteWidth = sizeof(TVertexType);

			HRESULT result;
			D3D11_BUFFER_DESC vertexBufferDesc;
			D3D11_SUBRESOURCE_DATA vertexData;
			
			//Setup the description of the static vertex buffer
			vertexBufferDesc.Usage = (mIsVBDynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
			vertexBufferDesc.ByteWidth = mVertexByteWidth * mVertexCount;
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = (mIsVBDynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
			vertexBufferDesc.MiscFlags = 0;
			vertexBufferDesc.StructureByteStride = 0;
			
			// Give the subresource a pointer to the vertex data
			vertexData.pSysMem = &vec[0];
			vertexData.SysMemPitch = 0;
			vertexData.SysMemSlicePitch = 0;
			
			//Create the vertex buffer
			result = mD3D->GetDevice11()->CreateBuffer(&vertexBufferDesc, &vertexData, &mVertexBuffer);
			if (FAILED(result)) return;
		}
		
		template<class TVertexType>
		void ReBuildVertexBuffer(vector<TVertexType> vec)
		{
			if (!mIsVBDynamic) return;
			
			D3D11_MAPPED_SUBRESOURCE resource;
			mD3D->GetDeviceContext()->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
			memcpy(resource.pData, &vec[0], mVertexByteWidth * mVertexCount);
			mD3D->GetDeviceContext()->Unmap(mVertexBuffer, 0);
		}

		void BuildIndexBuffer();
		
		void BuildInstanceBuffer();
		
		void ReBuildInstanceBuffer();
	};
}
//===============================================================================================================================
//===============================================================================================================================
#endif//__MESHATTRIBUTES_H