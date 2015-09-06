#include "OceanMesh.h"
#include "TextureManager.h"
//===============================================================================================================================
//===============================================================================================================================
OceanMesh::OceanMesh(D3D* d3d, OceanParameters params)
:	m_pD3DSystem(d3d)
,	mOceanParameters(params)
,	m_lods(0)
{
	Init();
}
//===============================================================================================================================
ZShadeSandboxMesh::MeshAttributes* OceanMesh::Attributes()
{
	return mAttributes;
}
//===============================================================================================================================
void OceanMesh::Init()
{
	mAttributes = new ZShadeSandboxMesh::MeshAttributes();
	mAttributes->mD3D = m_pD3DSystem;
	
	//
	// Vertex Buffer
	//
	
	mAttributes->mVertexCount = (mOceanParameters.g_MeshDimention + 1) * (mOceanParameters.g_MeshDimention + 1);
	
	mAttributes->mVerticesPos.resize(mAttributes->mVertexCount);
	
	//std::unique_ptr<XMFLOAT3[]> pV(new XMFLOAT3[mAttributes->mVertexCount]);
	//assert(pV);

	int x, z;
	for (z = 0; z <= mOceanParameters.g_MeshDimention; z++)
	{
		for (x = 0; x <= mOceanParameters.g_MeshDimention; x++)
		{
			mAttributes->mVerticesPos[z * (mOceanParameters.g_MeshDimention + 1) + x].position = XMFLOAT3(x, mOceanParameters.g_SeaLevel, z);
			//pV[z * (mOceanParameters.g_MapDimension + 1) + x] = XMFLOAT3(x, mOceanParameters.g_SeaLevel, z);
		}
	}
	
	/*for (int i = 0; i < mAttributes->mVertexCount; i++)
	{
		mAttributes->mVerticesPos[i].position = pV[i];
	}*/

	// Make this vertex buffer immutable
	mAttributes->BuildVertexBuffer(ZShadeSandboxMesh::EVertexType::Type::VT_Pos, false);
	
	//
	// Index Buffer
	//
	
	// The index numbers for all mesh LODs (up to 256x256)
	const int index_size_lookup[] = {0, 0, 4284, 18828, 69444, 254412, 956916, 3689820, 14464836};

	memset(&m_mesh_patterns[0][0][0][0][0], 0, sizeof(m_mesh_patterns));

	m_lods = 0;
	
	for (int i = mOceanParameters.g_MeshDimention; i > 1; i >>= 1)
	{
		m_lods++;
	}
	
	// Generate patch meshes. Each patch contains two parts: the inner mesh which is a regular
	// grids in a triangle strip. The boundary mesh is constructed w.r.t. the edge degrees to
	// meet water-tight requirement.
	UINT* index_array = new UINT[index_size_lookup[m_lods]];
	assert(index_array);
	//std::unique_ptr<DWORD[]> index_array(new DWORD[index_size_lookup[m_lods]]);

	int offset = 0;
	int level_size = mOceanParameters.g_MeshDimention;
	
	// Enumerate patterns
	for (int level = 0; level <= m_lods - 2; level ++)
	{
		int left_degree = level_size;
		
		for (int left_type = 0; left_type < 3; left_type ++)
		{
			int right_degree = level_size;
			
			for (int right_type = 0; right_type < 3; right_type ++)
			{
				int bottom_degree = level_size;
				
				for (int bottom_type = 0; bottom_type < 3; bottom_type ++)
				{
					int top_degree = level_size;
					
					for (int top_type = 0; top_type < 3; top_type ++)
					{
						QuadRenderParam* pattern = &m_mesh_patterns[level][left_type][right_type][bottom_type][top_type];
						
						// Inner mesh (triangle strip)
						RECT inner_rect;
						inner_rect.left   = (left_degree   == level_size) ? 0 : 1;
						inner_rect.right  = (right_degree  == level_size) ? level_size : level_size - 1;
						inner_rect.bottom = (bottom_degree == level_size) ? 0 : 1;
						inner_rect.top    = (top_degree    == level_size) ? level_size : level_size - 1;
						
						int num_new_indices = generateInnerMesh(inner_rect, index_array + offset);
						
						pattern->inner_start_index = offset;
						pattern->num_inner_verts = (level_size + 1) * (level_size + 1);
						pattern->num_inner_faces = num_new_indices - 2;
						offset += num_new_indices;
						
						// Boundary mesh (triangle list)
						int l_degree = (left_degree   == level_size) ? 0 : left_degree;
						int r_degree = (right_degree  == level_size) ? 0 : right_degree;
						int b_degree = (bottom_degree == level_size) ? 0 : bottom_degree;
						int t_degree = (top_degree    == level_size) ? 0 : top_degree;
						
						RECT outer_rect = {0, level_size, level_size, 0};
						
						num_new_indices = generateBoundaryMesh(l_degree, r_degree, b_degree, t_degree, outer_rect, index_array + offset);
						
						pattern->boundary_start_index = offset;
						pattern->num_boundary_verts = (level_size + 1) * (level_size + 1);
						pattern->num_boundary_faces = num_new_indices / 3;
						offset += num_new_indices;
						
						top_degree /= 2;
					}
					
					bottom_degree /= 2;
				}
				
				right_degree /= 2;
			}
			
			left_degree /= 2;
		}
		
		level_size /= 2;
	}
	
	assert(offset == index_size_lookup[m_lods]);
	
	mAttributes->mIndexCount = index_size_lookup[m_lods];
	
	mAttributes->mIndices.resize(mAttributes->mIndexCount);
	
	for (int i = 0; i < mAttributes->mIndexCount; i++)
	{
		mAttributes->mIndices[i] = index_array[i];
	}
	
	mAttributes->BuildIndexBuffer();
}
//===============================================================================================================================
int OceanMesh::generateBoundaryMesh(int left_degree, int right_degree, int bottom_degree, int top_degree, RECT vert_rect, UINT* output)
{
	// Triangle list for bottom boundary
	int i, j;
	int counter = 0;
	int width = vert_rect.right - vert_rect.left;
	
	if (bottom_degree > 0)
	{
		int b_step = width / bottom_degree;
		
		for (i = 0; i < width; i += b_step)
		{
			output[counter++] = MESH_INDEX_2D(i, 0);
			output[counter++] = MESH_INDEX_2D(i + b_step / 2, 1);
			output[counter++] = MESH_INDEX_2D(i + b_step, 0);
			
			for (j = 0; j < b_step / 2; j ++)
			{
				if (i == 0 && j == 0 && left_degree > 0)
				{
					continue;
				}
				
				output[counter++] = MESH_INDEX_2D(i, 0);
				output[counter++] = MESH_INDEX_2D(i + j, 1);
				output[counter++] = MESH_INDEX_2D(i + j + 1, 1);
			}

			for (j = b_step / 2; j < b_step; j ++)
			{
				if (i == width - b_step && j == b_step - 1 && right_degree > 0)
				{
					continue;
				}
				
				output[counter++] = MESH_INDEX_2D(i + b_step, 0);
				output[counter++] = MESH_INDEX_2D(i + j, 1);
				output[counter++] = MESH_INDEX_2D(i + j + 1, 1);
			}
		}
	}
	
	// Right boundary
	int height = vert_rect.top - vert_rect.bottom;
	
	if (right_degree > 0)
	{
		int r_step = height / right_degree;

		for (i = 0; i < height; i += r_step)
		{
			output[counter++] = MESH_INDEX_2D(width, i);
			output[counter++] = MESH_INDEX_2D(width - 1, i + r_step / 2);
			output[counter++] = MESH_INDEX_2D(width, i + r_step);

			for (j = 0; j < r_step / 2; j ++)
			{
				if (i == 0 && j == 0 && bottom_degree > 0)
				{
					continue;
				}

				output[counter++] = MESH_INDEX_2D(width, i);
				output[counter++] = MESH_INDEX_2D(width - 1, i + j);
				output[counter++] = MESH_INDEX_2D(width - 1, i + j + 1);
			}

			for (j = r_step / 2; j < r_step; j ++)
			{
				if (i == height - r_step && j == r_step - 1 && top_degree > 0)
				{
					continue;
				}

				output[counter++] = MESH_INDEX_2D(width, i + r_step);
				output[counter++] = MESH_INDEX_2D(width - 1, i + j);
				output[counter++] = MESH_INDEX_2D(width - 1, i + j + 1);
			}
		}
	}

	// Top boundary
	if (top_degree > 0)
	{
		int t_step = width / top_degree;

		for (i = 0; i < width; i += t_step)
		{
			output[counter++] = MESH_INDEX_2D(i, height);
			output[counter++] = MESH_INDEX_2D(i + t_step / 2, height - 1);
			output[counter++] = MESH_INDEX_2D(i + t_step, height);

			for (j = 0; j < t_step / 2; j ++)
			{
				if (i == 0 && j == 0 && left_degree > 0)
				{
					continue;
				}

				output[counter++] = MESH_INDEX_2D(i, height);
				output[counter++] = MESH_INDEX_2D(i + j, height - 1);
				output[counter++] = MESH_INDEX_2D(i + j + 1, height - 1);
			}

			for (j = t_step / 2; j < t_step; j ++)
			{
				if (i == width - t_step && j == t_step - 1 && right_degree > 0)
				{
					continue;
				}

				output[counter++] = MESH_INDEX_2D(i + t_step, height);
				output[counter++] = MESH_INDEX_2D(i + j, height - 1);
				output[counter++] = MESH_INDEX_2D(i + j + 1, height - 1);
			}
		}
	}

	// Left boundary
	if (left_degree > 0)
	{
		int l_step = height / left_degree;

		for (i = 0; i < height; i += l_step)
		{
			output[counter++] = MESH_INDEX_2D(0, i);
			output[counter++] = MESH_INDEX_2D(1, i + l_step / 2);
			output[counter++] = MESH_INDEX_2D(0, i + l_step);

			for (j = 0; j < l_step / 2; j ++)
			{
				if (i == 0 && j == 0 && bottom_degree > 0)
				{
					continue;
				}

				output[counter++] = MESH_INDEX_2D(0, i);
				output[counter++] = MESH_INDEX_2D(1, i + j);
				output[counter++] = MESH_INDEX_2D(1, i + j + 1);
			}

			for (j = l_step / 2; j < l_step; j ++)
			{
				if (i == height - l_step && j == l_step - 1 && top_degree > 0)
				{
					continue;
				}

				output[counter++] = MESH_INDEX_2D(0, i + l_step);
				output[counter++] = MESH_INDEX_2D(1, i + j);
				output[counter++] = MESH_INDEX_2D(1, i + j + 1);
			}
		}
	}

	return counter;
}
//===============================================================================================================================
int OceanMesh::generateInnerMesh(RECT vert_rect, UINT* output)
{
	int i, j;
	int counter = 0;
	int width = vert_rect.right - vert_rect.left;
	int height = vert_rect.top - vert_rect.bottom;

	bool reverse = false;
	for (i = 0; i < height; i++)
	{
		if (reverse == false)
		{
			output[counter++] = MESH_INDEX_2D(0, i);
			output[counter++] = MESH_INDEX_2D(0, i + 1);
			
			for (j = 0; j < width; j++)
			{
				output[counter++] = MESH_INDEX_2D(j + 1, i);
				output[counter++] = MESH_INDEX_2D(j + 1, i + 1);
			}
		}
		else
		{
			output[counter++] = MESH_INDEX_2D(width, i);
			output[counter++] = MESH_INDEX_2D(width, i + 1);
			
			for (j = width - 1; j >= 0; j--)
			{
				output[counter++] = MESH_INDEX_2D(j, i);
				output[counter++] = MESH_INDEX_2D(j, i + 1);
			}
		}

		reverse = !reverse;
	}

	return counter;
}
//===============================================================================================================================
OceanMesh::QuadRenderParam OceanMesh::CheckLookupTable(int lod, int left, int right, int bottom, int top)
{
	return m_mesh_patterns[lod][left][right][bottom][top];
}
//===============================================================================================================================