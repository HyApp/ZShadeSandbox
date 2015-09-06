#include "OceanRenderer.h"
#include "TextureManager.h"
// https://github.com/sfpgmr/sfmf2/blob/master/sfmf2/csocean/csocean_renderer.cpp
//===============================================================================================================================
//===============================================================================================================================
OceanRenderer::OceanRenderer(D3D* d3d, OceanParameters params)
:	m_pD3DSystem(d3d)
,	mOceanParameters(params)
{
	Init();
}
//===============================================================================================================================
void OceanRenderer::Init()
{
	// The wave texture names should be in ocean params so the editor can use any texture
	mSurfaceShader = new OceanSurfaceShader(m_pD3DSystem);
	mSurfaceShader->SetWaveMap0(TextureManager::Instance()->GetTexture("Textures\\wave0.dds"));
	mSurfaceShader->SetWaveMap1(TextureManager::Instance()->GetTexture("Textures\\wave1.dds"));
	
	// Create the mesh
	mMesh = new OceanMesh(m_pD3DSystem, mOceanParameters);
}
//===============================================================================================================================
void OceanRenderer::SetReflectionMap(ID3D11ShaderResourceView* srv)
{
	mSurfaceShader->SetReflectionMap(srv);
}
//==============================================================================================================================
void OceanRenderer::SetRefractionMap(ID3D11ShaderResourceView* srv)
{
	mSurfaceShader->SetRefractionMap(srv);
}
//==============================================================================================================================
void OceanRenderer::SetDisplacementMap(ID3D11ShaderResourceView* srv)
{
	mSurfaceShader->SetTexDisplacement(srv);
}
//==============================================================================================================================
void OceanRenderer::SetGradientMap(ID3D11ShaderResourceView* srv)
{
	mSurfaceShader->SetTexGradient(srv);
}
//==============================================================================================================================
void OceanRenderer::SetPerlinMap(ID3D11ShaderResourceView* srv)
{
	mSurfaceShader->SetTexPerlin(srv);
}
//==============================================================================================================================
void OceanRenderer::SetWireframe(bool wire)
{
	mSurfaceShader->Wireframe() = wire;
}
//==============================================================================================================================
void OceanRenderer::Render(Camera* camera)
{
	//
	// Build the rendering list for the ocean mesh
	//
	
	if (m_render_list.size() > 0)
	{
		m_render_list.clear();
	}
	
	float ocean_extent = mOceanParameters.g_PatchLength * (1 << mOceanParameters.g_FurthestCover);
	
	OceanMesh::QuadNode root_node = { XMFLOAT2(-ocean_extent * 0.5f, -ocean_extent * 0.5f), ocean_extent, 0, { -1, -1, -1, -1 } };
	
	BuildNodeList( root_node, camera );
	
	//
	// Setup matrices
	//
	
	ZShadeSandboxMath::XMMatrix view = camera->View();
	ZShadeSandboxMath::XMMatrix reflView = camera->ReflectionView();
	ZShadeSandboxMath::XMMatrix proj = camera->Proj();
	
	ZShadeSandboxMath::XMMatrix matView = view;// ZShadeSandboxMath::XMMatrix(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1) * view;
	ZShadeSandboxMath::XMMatrix matReflView = reflView;// ZShadeSandboxMath::XMMatrix(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1) * reflView;
	
	//
	// Set Shaders
	//
	
	mSurfaceShader->SetVertexShader();
	
	if (!mSurfaceShader->Wireframe())
	{
		mSurfaceShader->SwitchTo( "OceanSurfacePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL );
	}
	else
	{
		mSurfaceShader->SwitchTo("OceanSurfaceWireframePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	}
	
	mSurfaceShader->SetPixelShader();
	
	//
	// Set Shader resource textures
	//
	
	mSurfaceShader->BindTextures();
	
	//
	// Set Mesh Buffers
	//
	
	mMesh->Attributes()->SetBuffers();

	mSurfaceShader->SetInputLayout("OceanSurfaceShader");
	
	//
	// Set solid rendering state when not in wireframe mode
	//
	
	if (!mSurfaceShader->Wireframe())
	{
		m_pD3DSystem->TurnOffCulling();
	}
	
	//
	// Set shading constants
	//
	
	mSurfaceShader->SetShadingBuffer(mOceanParameters);
	
	//
	// Assume ocean is at (0, 0, 0)
	//
	
	for (int i = 0; i < (int)m_render_list.size(); i++)
	{
		OceanMesh::QuadNode& node = m_render_list[i];
		
		// Only render leaves
		if (!IsLeaf(node))
		{
			continue;
		}
		
		// Check adjacent patches and select mesh pattern
		OceanMesh::QuadRenderParam& render_param = SelectMeshPattern(node);
		
		// Find the right LOD to render
		int level_size = mOceanParameters.g_MeshDimention;
		
		for (int lod = 0; lod < node.lod; lod++)
		{
			level_size >>= 1;
		}
		
		//
		// Perform per call matrix calculations for the leaf
		//
		
		ZShadeSandboxMath::XMMatrix local;
		local.Scale(node.length / level_size, 0, node.length / level_size);
		
		mSurfaceShader->SetPerCallBuffer(mOceanParameters, node.bottom_left, local, matReflView, matView, proj);
		
		//
		// Perform drawing of the leaf
		//
		
		// Draw inner mesh
		if (render_param.num_inner_faces > 0)
		{
			mMesh->Attributes()->SetPrimitiveTopology(ZShadeSandboxMesh::ERenderType::eTriangleStrip);
			mSurfaceShader->RenderIndex11(render_param.inner_start_index, render_param.num_inner_faces + 2);
		}
		
		// Draw boundary mesh
		if (render_param.num_boundary_faces > 0)
		{
			mMesh->Attributes()->SetPrimitiveTopology(ZShadeSandboxMesh::ERenderType::eTriangleList);
			mSurfaceShader->RenderIndex11(render_param.boundary_start_index, render_param.num_boundary_faces * 3);
		}
	}
	
	//
	// Unbind the textures so they don't get set somewhere else they should not be
	//
	
	mSurfaceShader->UnbindTextures();
}
//==============================================================================================================================
int OceanRenderer::BuildNodeList(OceanMesh::QuadNode& quad_node, Camera* camera)
{
	//
	// Check against view frustum
	//
	
	if (!CheckNodeVisibility(quad_node, camera))
	{
		return -1;
	}
	
	//
	// Estimate the minimum grid coverage based on the size of the screen
	//
	
	UINT vp_count = 1;
	D3D11_VIEWPORT vp;
	m_pD3DSystem->GetDeviceContext()->RSGetViewports(&vp_count, &vp);
	
	float minimum_coverage = EstimateGridCoverage(quad_node, camera, (float)(vp.Width * vp.Height));
	
	//
	// Recursively attach sub-nodes.
	//
	
	bool visible = true;
	
	if (minimum_coverage > mOceanParameters.g_UpperGridCoverage && quad_node.length > mOceanParameters.g_PatchLength)
	{
		// Recursive rendering for sub-quads.
		OceanMesh::QuadNode sub_node_0 = {quad_node.bottom_left, quad_node.length / 2, 0, {-1, -1, -1, -1}};
		quad_node.sub_node[0] = BuildNodeList(sub_node_0, camera);
		
		OceanMesh::QuadNode sub_node_1 = { ZShadeSandboxMath::XMMath2(quad_node.bottom_left) + ZShadeSandboxMath::XMMath2(quad_node.length / 2, 0), quad_node.length / 2, 0, { -1, -1, -1, -1 } };
		quad_node.sub_node[1] = BuildNodeList(sub_node_1, camera);
		
		OceanMesh::QuadNode sub_node_2 = { ZShadeSandboxMath::XMMath2(quad_node.bottom_left) + ZShadeSandboxMath::XMMath2(quad_node.length / 2, quad_node.length / 2), quad_node.length / 2, 0, { -1, -1, -1, -1 } };
		quad_node.sub_node[2] = BuildNodeList(sub_node_2, camera);
		
		OceanMesh::QuadNode sub_node_3 = { ZShadeSandboxMath::XMMath2(quad_node.bottom_left) + ZShadeSandboxMath::XMMath2(0, quad_node.length / 2), quad_node.length / 2, 0, { -1, -1, -1, -1 } };
		quad_node.sub_node[3] = BuildNodeList(sub_node_3, camera);
		
		visible = !IsLeaf(quad_node);
	}
	
	if (visible)
	{
		// Estimate mesh LOD
		int lod = 0;
		for (lod = 0; lod < mMesh->Lods() - 1; lod++)
		{
			if (minimum_coverage > mOceanParameters.g_UpperGridCoverage)
			{
				break;
			}
			
			minimum_coverage *= 4;
		}
		
		// We don't use 1x1 and 2x2 patch. So the highest level is g_Lods - 2.
		quad_node.lod = min(lod, mMesh->Lods() - 2);
	}
	else
	{
		// Patches are not visible
		return -1;
	}
	
	// Insert into the list
	int position = (int)m_render_list.size();
	
	m_render_list.push_back(quad_node);
	
	return position;
}
//==============================================================================================================================
bool OceanRenderer::CheckNodeVisibility(const OceanMesh::QuadNode& quad_node, Camera* camera)
{
	//
	// Plane equation setup
	//
	
	ZShadeSandboxMath::XMMatrix view = camera->View();
	ZShadeSandboxMath::XMMatrix proj = camera->Proj();

	proj.UpdateFields();
	
	ZShadeSandboxMath::XMMatrix matView = ZShadeSandboxMath::XMMatrix(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1) * view;
	
	float fov_x = atan(1.0f / proj(0, 0));
	float fov_y = atan(1.0f / proj(1, 1));
	
	ZShadeSandboxMath::XMMath4 plane_left(cos(fov_x), 0, sin(fov_x), 0);
	ZShadeSandboxMath::XMMath4 plane_right(-cos(fov_x), 0, sin(fov_x), 0);
	ZShadeSandboxMath::XMMath4 plane_bottom(0, cos(fov_y), sin(fov_y), 0);
	ZShadeSandboxMath::XMMath4 plane_top(0, -cos(fov_y), sin(fov_y), 0);
	
	//
	// Test quad corners against the view frustum in view space
	//
	
	ZShadeSandboxMath::XMMath4 corners[4];
	
	corners[0] = ZShadeSandboxMath::XMMath4(quad_node.bottom_left.x, quad_node.bottom_left.y, 0, 1);
	corners[1] = corners[0] + ZShadeSandboxMath::XMMath4(quad_node.length, 0, 0, 0);
	corners[2] = corners[0] + ZShadeSandboxMath::XMMath4(quad_node.length, quad_node.length, 0, 0);
	corners[3] = corners[0] + ZShadeSandboxMath::XMMath4(0, quad_node.length, 0, 0);
	
	corners[0] = (ZShadeSandboxMath::XMMath4)corners[0].Transform(matView);
	corners[1] = (ZShadeSandboxMath::XMMath4)corners[1].Transform(matView);
	corners[2] = (ZShadeSandboxMath::XMMath4)corners[2].Transform(matView);
	corners[3] = (ZShadeSandboxMath::XMMath4)corners[3].Transform(matView);
	
	float temp_y = corners[0].y;
	corners[0].y = corners[0].z;
	corners[0].z = temp_y;

	temp_y = corners[1].y;
	corners[1].y = corners[1].z;
	corners[1].z = temp_y;

	temp_y = corners[2].y;
	corners[2].y = corners[2].z;
	corners[2].z = temp_y;

	temp_y = corners[3].y;
	corners[3].y = corners[3].z;
	corners[3].z = temp_y;

	int test_value = -2000;
	
	// Test against eye plane
	if (corners[0].y < test_value && corners[1].y < test_value && corners[2].y < test_value && corners[3].y < test_value)
	{
		return false;
	}
	
	// Test against left plane
	float dist_0 = corners[0].Dot(plane_left);
	float dist_1 = corners[1].Dot(plane_left);
	float dist_2 = corners[2].Dot(plane_left);
	float dist_3 = corners[3].Dot(plane_left);
	if (dist_0 < test_value && dist_1 < test_value && dist_2 < test_value && dist_3 < test_value)
	{
		return false;
	}
	
	// Test against right plane
	dist_0 = corners[0].Dot(plane_right);
	dist_1 = corners[1].Dot(plane_right);
	dist_2 = corners[2].Dot(plane_right);
	dist_3 = corners[3].Dot(plane_right);
	if (dist_0 < test_value && dist_1 < test_value && dist_2 < test_value && dist_3 < test_value)
	{
		return false;
	}
	
	// Test against bottom plane
	dist_0 = corners[0].Dot(plane_bottom);
	dist_1 = corners[1].Dot(plane_bottom);
	dist_2 = corners[2].Dot(plane_bottom);
	dist_3 = corners[3].Dot(plane_bottom);
	if (dist_0 < test_value && dist_1 < test_value && dist_2 < test_value && dist_3 < test_value)
	{
		return false;
	}
	
	// Test against top plane
	dist_0 = corners[0].Dot(plane_top);
	dist_1 = corners[1].Dot(plane_top);
	dist_2 = corners[2].Dot(plane_top);
	dist_3 = corners[3].Dot(plane_top);
	if (dist_0 < test_value && dist_1 < test_value && dist_2 < test_value && dist_3 < test_value)
	{
		return false;
	}
	
	return true;
}
//==============================================================================================================================
OceanMesh::QuadRenderParam& OceanRenderer::SelectMeshPattern(const OceanMesh::QuadNode& quad_node)
{
	//
	// Check 4 adjacent quads to this quad
	//
	
	ZShadeSandboxMath::XMMath2 point_left = ZShadeSandboxMath::XMMath2(quad_node.bottom_left.x, quad_node.bottom_left.y) + ZShadeSandboxMath::XMMath2(-mOceanParameters.g_PatchLength * 0.5f, quad_node.length * 0.5f);
	ZShadeSandboxMath::XMMath2 point_right = ZShadeSandboxMath::XMMath2(quad_node.bottom_left) + ZShadeSandboxMath::XMMath2(quad_node.length + mOceanParameters.g_PatchLength * 0.5f, quad_node.length * 0.5f);
	ZShadeSandboxMath::XMMath2 point_bottom = ZShadeSandboxMath::XMMath2(quad_node.bottom_left) + ZShadeSandboxMath::XMMath2(quad_node.length * 0.5f, -mOceanParameters.g_PatchLength * 0.5f);
	ZShadeSandboxMath::XMMath2 point_top = ZShadeSandboxMath::XMMath2(quad_node.bottom_left) + ZShadeSandboxMath::XMMath2(quad_node.length * 0.5f, quad_node.length + mOceanParameters.g_PatchLength * 0.5f);
	
	int left_adj_index = SearchLeaf(m_render_list, point_left);
	int right_adj_index = SearchLeaf(m_render_list, point_right);
	int bottom_adj_index = SearchLeaf(m_render_list, point_bottom);
	int top_adj_index = SearchLeaf(m_render_list, point_top);
	
	int left_type = 0;
	int right_type = 0;
	int bottom_type = 0;
	int top_type = 0;
	
	if (left_adj_index != -1 && m_render_list[left_adj_index].length > quad_node.length * 0.999f)
	{
		OceanMesh::QuadNode adj_node = m_render_list[left_adj_index];
		
		float scale = adj_node.length / quad_node.length * ((int)mOceanParameters.g_MeshDimention >> quad_node.lod) / ((int)mOceanParameters.g_MeshDimention >> adj_node.lod);
		
		if (scale > 3.999f)
		{
			left_type = 2;
		}
		else if (scale > 1.999f)
		{
			left_type = 1;
		}
	}
	
	if (right_adj_index != -1 && m_render_list[right_adj_index].length > quad_node.length * 0.999f)
	{
		OceanMesh::QuadNode adj_node = m_render_list[right_adj_index];
		
		float scale = adj_node.length / quad_node.length * ((int)mOceanParameters.g_MeshDimention >> quad_node.lod) / ((int)mOceanParameters.g_MeshDimention >> adj_node.lod);
		
		if (scale > 3.999f)
		{
			right_type = 2;
		}
		else if (scale > 1.999f)
		{
			right_type = 1;
		}
	}
	
	if (bottom_adj_index != -1 && m_render_list[bottom_adj_index].length > quad_node.length * 0.999f)
	{
		OceanMesh::QuadNode adj_node = m_render_list[bottom_adj_index];
		
		float scale = adj_node.length / quad_node.length * ((int)mOceanParameters.g_MeshDimention >> quad_node.lod) / ((int)mOceanParameters.g_MeshDimention >> adj_node.lod);
		
		if (scale > 3.999f)
		{
			bottom_type = 2;
		}
		else if (scale > 1.999f)
		{
			bottom_type = 1;
		}
	}
	
	if (top_adj_index != -1 && m_render_list[top_adj_index].length > quad_node.length * 0.999f)
	{
		OceanMesh::QuadNode adj_node = m_render_list[top_adj_index];
		
		float scale = adj_node.length / quad_node.length * ((int)mOceanParameters.g_MeshDimention >> quad_node.lod) / ((int)mOceanParameters.g_MeshDimention >> adj_node.lod);
		
		if (scale > 3.999f)
		{
			top_type = 2;
		}
		else if (scale > 1.999f)
		{
			top_type = 1;
		}
	}
	
	// Check lookup table, [L][R][B][T]
	return mMesh->CheckLookupTable(quad_node.lod, left_type, right_type, bottom_type, top_type);
}
//==============================================================================================================================
float OceanRenderer::EstimateGridCoverage(const OceanMesh::QuadNode& quad_node, Camera* camera, float screen_area)
{
	// Estimate projected area

	// Test 16 points on the quad and find out the biggest one.
	const static float sample_pos[16][2] =
	{
		{0, 0},
		{0, 1},
		{1, 0},
		{1, 1},
		{0.5f, 0.333f},
		{0.25f, 0.667f},
		{0.75f, 0.111f},
		{0.125f, 0.444f},
		{0.625f, 0.778f},
		{0.375f, 0.222f},
		{0.875f, 0.556f},
		{0.0625f, 0.889f},
		{0.5625f, 0.037f},
		{0.3125f, 0.37f},
		{0.8125f, 0.704f},
		{0.1875f, 0.148f},
	};
	
	ZShadeSandboxMath::XMMatrix view = camera->View();
	ZShadeSandboxMath::XMMatrix proj = camera->Proj();
	
	proj.UpdateFields();

	ZShadeSandboxMath::XMMath3 eye(camera->Position().x, camera->Position().y, camera->Position().z);
	
	float grid_len_world = quad_node.length / mOceanParameters.g_MeshDimention;
	
	float max_area_proj = 0;
	
	for (int i = 0; i < 16; i++)
	{
		ZShadeSandboxMath::XMMath3 testPoint(quad_node.bottom_left.x + quad_node.length * sample_pos[i][0],
										     0,
										     quad_node.bottom_left.y + quad_node.length * sample_pos[i][1]);
		
		ZShadeSandboxMath::XMMath3 eye_vec = testPoint - eye;
		
		float dist = eye_vec.Length();
		
		float area_world = grid_len_world * grid_len_world;
		float area_proj = (area_world * proj(0, 0) * proj(1, 1)) / dist * dist;
		
		if (max_area_proj < area_proj)
		{
			max_area_proj = area_proj;
		}
	}
	
	float pixel_coverage = max_area_proj * screen_area * 0.25f;

	return pixel_coverage;
}
//==============================================================================================================================
bool OceanRenderer::IsLeaf(const OceanMesh::QuadNode& quad_node)
{
	return (quad_node.sub_node[0] == -1 && quad_node.sub_node[1] == -1 && quad_node.sub_node[2] == -1 && quad_node.sub_node[3] == -1);
}
//==============================================================================================================================
int OceanRenderer::SearchLeaf(const vector<OceanMesh::QuadNode>& node_list, const ZShadeSandboxMath::XMMath2& point)
{
	int index = -1;
	
	int size = (int)node_list.size();
	
	OceanMesh::QuadNode node = node_list[size - 1];
	
	while (!IsLeaf(node))
	{
		bool found = false;
		
		for (int i = 0; i < 4; i++)
		{
			index = node.sub_node[i];
			
			if (index == -1)
			{
				continue;
			}
			
			OceanMesh::QuadNode sub_node = node_list[index];
			
			if (point.x >= sub_node.bottom_left.x && point.x <= sub_node.bottom_left.x + sub_node.length &&
				point.y >= sub_node.bottom_left.y && point.y <= sub_node.bottom_left.y + sub_node.length)
			{
				node = sub_node;
				found = true;
				break;
			}
		}
		
		if (!found)
		{
			return -1;
		}
	}
	
	return index;
}
//==============================================================================================================================
