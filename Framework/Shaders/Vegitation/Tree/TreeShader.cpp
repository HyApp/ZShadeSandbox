#include "TreeShader.h"
#include "ConstantBuffer.h"
#include "ZMath.h"
//==============================================================================================================================
TreeShader::TreeShader(D3D* d3d)
:	ZShadeSandboxShader::Shader(d3d)
,	fBillTreeWidth(0)
,	fBillTreeHeight(0)
,	fFarPlane(0)
{
	Initialize();
}
//==============================================================================================================================
TreeShader::TreeShader(const TreeShader& other)
:	ZShadeSandboxShader::Shader(other)
{
}
//==============================================================================================================================
TreeShader::~TreeShader()
{
}
//==============================================================================================================================
bool TreeShader::Initialize()
{
	ConstantBuffer<cbTreeBuffer> treeCB(m_pD3DSystem);
	treeCB.Initialize(PAD16(sizeof(cbTreeBuffer)));
	m_pTreeBufferCB = treeCB.Buffer();
	
	ConstantBuffer<cbMatrixBuffer> matrixBufferCB(m_pD3DSystem);
	matrixBufferCB.Initialize(PAD16(sizeof(cbMatrixBuffer)));
	m_pMatrixBufferCB = matrixBufferCB.Buffer();
	
	D3D11_INPUT_ELEMENT_DESC leafLayout[7] =
	{
		{ "position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, NUM_LEAVES_PER_TREE },
		{ "INSTANCEROT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, NUM_LEAVES_PER_TREE },
		{ "INSTANCESCL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, NUM_LEAVES_PER_TREE }
	};
	
	ClearInputLayout();
	SetInputLayoutDesc("TreeShaderLeaf", leafLayout, 7);
	LoadVertexShader("TreeVS");
	LoadGeometryShader("TreeBillboardGS");
	LoadPixelShader("TreePS");
	LoadPixelShader("TreeWireframePS");
	AssignVertexShaderLayout("TreeShaderLeaf");
	
	// Still use the same vertex shader but create a new layout
	SetInputLayoutDesc("TreeShader", ZShadeSandboxMesh::VertexLayout::mesh_layout_pos_normal_tex_tan_instance_rot_scl, 7);
	AssignVertexShaderLayout("TreeShader");
	
	return true;
}
//==============================================================================================================================
void TreeShader::InitializeLeafBuffer(InternalTreeRenderParameters trp)
{
	ConstantBuffer<cbLeafBuffer> leafCB(m_pD3DSystem);
	leafCB.Initialize(PAD16(sizeof(cbLeafBuffer)));
	m_pLeafBufferCB = leafCB.Buffer();
	
	cbLeafBuffer cLB;
	
	for (int i = 0; i < NUM_LEAVES_PER_TREE; i++)
	{
		cLB.g_LeafOnTree[i] = ZShadeSandboxMath::ZMath::GMathMF(XMMatrixTranspose(trp.g_LeafOnTree[i]));
	}
	
	ID3D11DeviceContext* context = m_pD3DSystem->GetDeviceContext();
	
	D3D11_MAPPED_SUBRESOURCE mapped_res;
	context->Map(m_pLeafBufferCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
	{
		assert(mapped_res.pData);
		*(cbLeafBuffer*)mapped_res.pData = cLB;
	}
	context->Unmap(m_pLeafBufferCB, 0);
}
//==============================================================================================================================
void TreeShader::Shutdown()
{
	//ShutdownShader();
}
//==============================================================================================================================
void TreeShader::RenderBillboards
(	ZShadeSandboxMesh::MeshRenderParameters mrp
,	ID3D11ShaderResourceView* billboardTexture
)
{
	cbTreeBuffer cTB;
	cTB.g_EyePos = mrp.camera->Position();
	cTB.g_TreeBillWidth = fBillTreeWidth;
	cTB.g_TreeBillHeight = fBillTreeHeight;
	cTB.padding1 = 0;
	cTB.padding2 = 0;
	cTB.g_DiffuseColor = XMFLOAT4(0, 0, 0, 0);
	cTB.g_HasTexture = 1;
	cTB.g_HasNormalMap = 0;
	cTB.g_IsInstance = (mrp.useInstancing) ? 1 : 0;
	cTB.g_IsLeaf = 0;
	// Map the tree parameter constants
	{
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		m_pD3DSystem->GetDeviceContext()->Map(m_pTreeBufferCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		{
			assert(mapped_res.pData);
			*(cbTreeBuffer*)mapped_res.pData = cTB;
		}
		m_pD3DSystem->GetDeviceContext()->Unmap(m_pTreeBufferCB, 0);
	}
	
	cbMatrixBuffer cMB;
	cMB.g_matWorld = ZShadeSandboxMath::ZMath::GMathMF(mrp.world);
	cMB.g_matView = (mrp.specifyView) ? mrp.view : mrp.camera->View4x4();
	cMB.g_matProj = (mrp.specifyProj) ? mrp.proj : mrp.camera->Proj4x4();
	// Map the matrix parameter constants
	{
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		m_pD3DSystem->GetDeviceContext()->Map(m_pMatrixBufferCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		{
			assert(mapped_res.pData);
			*(cbMatrixBuffer*)mapped_res.pData = cMB;
		}
		m_pD3DSystem->GetDeviceContext()->Unmap(m_pMatrixBufferCB, 0);
	}
	
	ID3D11Buffer* constBuffers[3] = {m_pTreeBufferCB, m_pMatrixBufferCB, m_pLeafBufferCB};
	
	m_pD3DSystem->GetDeviceContext()->VSSetConstantBuffers(0, 3, constBuffers);
	m_pD3DSystem->GetDeviceContext()->GSSetConstantBuffers(0, 3, constBuffers);
	m_pD3DSystem->GetDeviceContext()->PSSetConstantBuffers(0, 3, constBuffers);
	
	ID3D11ShaderResourceView* ps_srvs[1] = { billboardTexture };
	ID3D11SamplerState* ps_samp[1] = { m_pD3DSystem->Linear() };
	
	m_pD3DSystem->TurnOffCulling();
	
	if (!m_Wireframe)
	{
		// Assign Texture

		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 1, ps_samp);
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);

		SwitchTo("TreePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	}
	else
	{
		SwitchTo("TreeWireframePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	}

	SwitchTo("TreeVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);

	SetInputLayout("TreeShader");
	
	SetVertexShader();
	SetGeometryShader();
	SetPixelShader();
	
	//Perform Drawing
	if (cTB.g_IsInstance == 1)
	{
		RenderDrawInstanced11(1, mrp.instanceCount);
	}
	else
	{
		RenderDraw11(1);
	}
	
	// Unbind
	if (!m_Wireframe)
	{
		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 1, ps_samp);
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);
	}
	
	m_pD3DSystem->GetDeviceContext()->GSSetShader(NULL, 0, 0);
	
	m_pD3DSystem->TurnOnCulling();
	
	// Reset topology back to triangles
	m_pD3DSystem->GetDeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
}
//==============================================================================================================================
void TreeShader::RenderAndSaveBillboard
(	int leafIndexCount
,	ZShadeSandboxMesh::MeshRenderParameters mrp
,	ID3D11ShaderResourceView* leafTexture
,	ZShadeSandboxMath::AABB treeAABB
,	ZShadeSandboxMesh::OBJMesh* treeMesh
,	std::string billboardTextureName
)
{
	ID3D11Device* device = m_pD3DSystem->GetDevice11();
	ID3D11DeviceContext* deviceContext = m_pD3DSystem->GetDeviceContext();
	
	// Get our billboards texture (save it as a file)
	ID3D11Texture2D* billRenderTargetTexture;
	ID3D11RenderTargetView* billRenderTargetView;
	
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	
	// Initialize the  texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	
	// Setup the texture description.
	// We will have the width of the texture be 512 pixels,
	// and the height of the texture depends on the height:width ratio of the bounding box (y/x)
	
	float treeWidth = treeAABB.vMax.x - treeAABB.vMin.x;	// Get length of AABB along x axis
	float treeHeight = treeAABB.vMax.y - treeAABB.vMin.y;	// Get length of AABB along y axis
	
	int tempHeight = (treeHeight / treeWidth) * 512;
	
	textureDesc.Width = 512;
	textureDesc.Height = tempHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	// Create the texture
	device->CreateTexture2D(&textureDesc, NULL, &billRenderTargetTexture);
	
	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	
	// Create the render target view.
	device->CreateRenderTargetView(billRenderTargetTexture, &renderTargetViewDesc, &billRenderTargetView);
	
	// Now we have to create the viewport that matches the render target
	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));
	
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = 512;
	vp.Height = tempHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	
	//Set the Viewport
	deviceContext->RSSetViewports(1, &vp);
	
	// Map camera information
	// We want the camera to be far enough away from the object so we get a picture of the entire object.
	// We do this by setting the cameras position to be at the center of the object, then moving back until we are
	// at the furthest z value (subtract minvertex.z) from the center, then subtract 1 so we know the entire object
	// will be in the cameras view
	XMFLOAT3 aabbCenter = treeAABB.Center();
	XMVECTOR billCamPosition = XMVectorSet(aabbCenter.x, aabbCenter.y , treeAABB.vMin.z - 20.0f, 0.0f);
	XMVECTOR billCamTarget = XMVectorSet(aabbCenter.x, aabbCenter.y, treeAABB.vMin.z, 0.0f);
	XMVECTOR billCamUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	
	//Set the View matrix
	XMMATRIX billView = XMMatrixLookAtLH( billCamPosition, billCamTarget, billCamUp );
	
	// Build an orthographic projection matrix
	// We want the edges of the projections view to be the width and height of the object, which is
	// defined by the AABB of the object
	XMMATRIX billProjection = XMMatrixOrthographicLH( treeWidth, treeHeight, 0.0f, 1000.0f );
	
	//Clear our render target
	float bgColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	deviceContext->ClearRenderTargetView(billRenderTargetView, bgColor);	
	deviceContext->ClearDepthStencilView(m_pD3DSystem->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	//constbuffPerFrame.light = light;
	//deviceContext->UpdateSubresource( cbPerFrameBuffer, 0, NULL, &constbuffPerFrame, 0, 0 );
	//deviceContext->PSSetConstantBuffers(0, 1, &cbPerFrameBuffer);	
	
	//Set our Render Target
	deviceContext->OMSetRenderTargets(1, &billRenderTargetView, m_pD3DSystem->GetDepthStencilView());
	
	//Set the default blend state (no blending) for opaque objects
	deviceContext->OMSetBlendState(0, 0, 0xffffffff);
	
	// Need to use Specify the matrices for the billboard
	mrp.world = XMMatrixTranspose(XMMatrixIdentity());
	mrp.specifyView = true;
	mrp.view = ZShadeSandboxMath::ZMath::GMathMF(XMMatrixTranspose(billView));
	mrp.specifyProj = true;
	mrp.proj = ZShadeSandboxMath::ZMath::GMathMF(XMMatrixTranspose(billProjection));
	
	// Draw INSTANCED Leaf Models
	RenderLeaves( leafIndexCount, mrp, leafTexture );
	
	// Only draw one tree
	mrp.instanceCount = 1;
	
	// Draw our tree instances
	treeMesh->Render(mrp);
	
	// Save the texture to a file
	TextureManager::Instance()->WriteToFile(billboardTextureName, billRenderTargetTexture);
	
	billRenderTargetTexture->Release();
	billRenderTargetView->Release();
}
//==============================================================================================================================
void TreeShader::RenderLeaves
(	int indexCount
,	ZShadeSandboxMesh::MeshRenderParameters mrp
,	ID3D11ShaderResourceView* texture
)
{
	cbTreeBuffer cTB;
	cTB.g_EyePos = mrp.camera->Position();
	cTB.g_TreeBillWidth = fBillTreeWidth;
	cTB.g_TreeBillHeight = fBillTreeHeight;
	cTB.padding1 = 0;
	cTB.padding2 = 0;
	cTB.g_DiffuseColor = XMFLOAT4(0, 0, 0, 0);
	cTB.g_HasTexture = 1;
	cTB.g_HasNormalMap = 0;
	cTB.g_IsInstance = (mrp.useInstancing) ? 1 : 0;
	cTB.g_IsLeaf = 1;
	// Map the tree parameter constants
	{
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		m_pD3DSystem->GetDeviceContext()->Map(m_pTreeBufferCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		{
			assert(mapped_res.pData);
			*(cbTreeBuffer*)mapped_res.pData = cTB;
		}
		m_pD3DSystem->GetDeviceContext()->Unmap(m_pTreeBufferCB, 0);
	}
	
	cbMatrixBuffer cMB;
	cMB.g_matWorld = ZShadeSandboxMath::ZMath::GMathMF(mrp.world);
	cMB.g_matView = (mrp.specifyView) ? mrp.view : mrp.camera->View4x4();
	cMB.g_matProj = (mrp.specifyProj) ? mrp.proj : mrp.camera->Proj4x4();
	// Map the matrix parameter constants
	{
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		m_pD3DSystem->GetDeviceContext()->Map(m_pMatrixBufferCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		{
			assert(mapped_res.pData);
			*(cbMatrixBuffer*)mapped_res.pData = cMB;
		}
		m_pD3DSystem->GetDeviceContext()->Unmap(m_pMatrixBufferCB, 0);
	}
	
	ID3D11Buffer* constBuffers[3] = {m_pTreeBufferCB, m_pMatrixBufferCB, m_pLeafBufferCB};
	
	m_pD3DSystem->GetDeviceContext()->VSSetConstantBuffers(0, 3, constBuffers);
	m_pD3DSystem->GetDeviceContext()->PSSetConstantBuffers(0, 3, constBuffers);
	
	ID3D11ShaderResourceView* ps_srvs[1] = { texture };
	ID3D11SamplerState* ps_samp[1] = { m_pD3DSystem->Linear() };
	
	m_pD3DSystem->TurnOffCulling();
	
	if (!m_Wireframe)
	{
		// Assign Texture

		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 1, ps_samp);
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);

		//if (mUseGBuffer)
		//{
		//	SwitchTo("TreeGBufferPS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
		//}
		//else
		{
			SwitchTo("TreePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
		}
	}
	else
	{
		//if (mUseGBuffer)
		//{
		//	SwitchTo("TreeGBufferWireframePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
		//}
		//else
		{
			SwitchTo("TreeWireframePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
		}
	}

	//if (mUseGBuffer)
	//{
	//	SwitchTo("TreeGBufferVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
	//}
	//else
	{
		SwitchTo("TreeVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
	}

	SetInputLayout("TreeShaderLeaf");

	SetVertexShader();
	SetPixelShader();

	//Perform Drawing
	if (cTB.g_IsInstance == 1)
	{
		RenderIndex11(indexCount, mrp.instanceCount);
	}
	else
	{
		RenderIndex11(indexCount);
	}
	
	// Unbind
	if (!m_Wireframe)
	{
		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 1, ps_samp);
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);
	}
	
	// Reset the default Input Layout
	SetInputLayout("TreeShader");
	
	m_pD3DSystem->TurnOnCulling();
}
//==============================================================================================================================
void TreeShader::Render
(	int indexStart
,	int indexCount
,	ZShadeSandboxMesh::MeshRenderParameters mrp
,	ZShadeSandboxLighting::ShaderMaterial* material
)
{
	ID3D11ShaderResourceView* diffuseArrayTexture = 0;
	ID3D11ShaderResourceView* diffuseTexture = 0;
	ID3D11ShaderResourceView* ambientTexture = 0;
	ID3D11ShaderResourceView* specularTexture = 0;
	ID3D11ShaderResourceView* emissiveTexture = 0;
	ID3D11ShaderResourceView* normalMapTexture = 0;
	ID3D11ShaderResourceView* blendMapTexture = 0;
	ID3D11ShaderResourceView* detailMapTexture = 0;
	ID3D11ShaderResourceView* alphaMapTexture = 0;
	ID3D11ShaderResourceView* shadowMapTexture = 0;
	ID3D11ShaderResourceView* ssaoTexture = 0;
	ID3D11ShaderResourceView* displacementMapTexture = 0;

	if (material == 0) return;

	material->GetTextures(
		diffuseArrayTexture,
		diffuseTexture,
		ambientTexture,
		specularTexture,
		emissiveTexture,
		normalMapTexture,
		blendMapTexture,
		detailMapTexture,
		alphaMapTexture,
		shadowMapTexture,
		ssaoTexture,
		displacementMapTexture
	);

	material->bUsingInstancing = mrp.useInstancing;

	material->fBlendAmount = mrp.blendAmount;

	cbTreeBuffer cTB;
	cTB.g_EyePos = mrp.camera->Position();
	cTB.g_TreeBillWidth = fBillTreeWidth;
	cTB.g_TreeBillHeight = fBillTreeHeight;
	cTB.padding1 = 0;
	cTB.padding2 = 0;
	cTB.g_DiffuseColor = XMFLOAT4(0, 0, 0, 0);
	cTB.g_HasTexture = 1;
	cTB.g_HasNormalMap = 0;
	cTB.g_IsInstance = (mrp.useInstancing) ? 1 : 0;
	cTB.g_IsLeaf = 0;
	// Map the tree parameter constants
	{
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		m_pD3DSystem->GetDeviceContext()->Map(m_pTreeBufferCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		{
			assert(mapped_res.pData);
			*(cbTreeBuffer*)mapped_res.pData = cTB;
		}
		m_pD3DSystem->GetDeviceContext()->Unmap(m_pTreeBufferCB, 0);
	}
	
	cbMatrixBuffer cMB;
	cMB.g_matWorld = ZShadeSandboxMath::ZMath::GMathMF(mrp.world);
	cMB.g_matView = (mrp.specifyView) ? mrp.view : mrp.camera->View4x4();
	cMB.g_matProj = (mrp.specifyProj) ? mrp.proj : mrp.camera->Proj4x4();
	// Map the matrix parameter constants
	{
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		m_pD3DSystem->GetDeviceContext()->Map(m_pMatrixBufferCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		{
			assert(mapped_res.pData);
			*(cbMatrixBuffer*)mapped_res.pData = cMB;
		}
		m_pD3DSystem->GetDeviceContext()->Unmap(m_pMatrixBufferCB, 0);
	}
	
	ID3D11Buffer* constBuffers[3] = {m_pTreeBufferCB, m_pMatrixBufferCB, m_pLeafBufferCB};
	
	m_pD3DSystem->GetDeviceContext()->VSSetConstantBuffers(0, 3, constBuffers);
	m_pD3DSystem->GetDeviceContext()->PSSetConstantBuffers(0, 3, constBuffers);
	
	ID3D11ShaderResourceView* ps_srvs[1] = { diffuseTexture }; // The diffuse texture for the tree
	ID3D11SamplerState* ps_samp[1] = { m_pD3DSystem->Linear() };
	
	m_pD3DSystem->TurnOffCulling();
	
	if (!m_Wireframe)
	{
		// Assign Texture

		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 1, ps_samp);
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);

		SwitchTo("TreePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	}
	else
	{
		SwitchTo("TreeWireframePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	}

	SwitchTo("TreeVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);

	SetInputLayout("TreeShader");

	SetVertexShader();
	SetPixelShader();
	
	//Perform Drawing
	if (cTB.g_IsInstance == 1)
	{
		RenderIndexInstanced11(indexStart, indexCount, mrp.instanceCount);
	}
	else
	{
		RenderIndex11(indexStart, indexCount);
	}
	
	// Unbind
	if (!m_Wireframe)
	{
		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 1, ps_samp);
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);
	}
	
	m_pD3DSystem->TurnOnCulling();
}
//==============================================================================================================================
/*void TreeShader::RenderFunc(int indexCount, ZShadeSandboxMesh::MeshRenderParameters mrp, ZShadeSandboxLighting::ShaderMaterial* material)
{
	cbWaterBuffer cWB;
	cbMatrixBuffer cMB;

	cMB.g_matWorld = ZShadeSandboxMath::ZMath::GMathMF(XMMatrixTranspose(mrp.world));
	cMB.g_matView = mrp.camera->View4x4();
	cMB.g_matReflView = mrp.camera->ReflectionView4x4();

	if (m_pD3DSystem->GetEngineOptions()->m_DimType == DimType::ZSHADE_2D)
		cMB.g_matProj = mrp.camera->Ortho4x4();
	else if (m_pD3DSystem->GetEngineOptions()->m_DimType == DimType::ZSHADE_3D)
		cMB.g_matProj = mrp.camera->Proj4x4();

	cWB.g_WaveHeight = mWaterParameters->g_WaveHeight;
	cWB.g_SpecularShininess = mWaterParameters->g_SpecularShininess;
	cWB.g_Time = mWaterParameters->g_Time;
	cWB.g_HalfCycle = mWaterParameters->g_HalfCycle;
	cWB.g_CamPos = mWaterParameters->g_CamPos;
	cWB.g_FlowMapOffset0 = mWaterParameters->g_FlowMapOffset0;
	cWB.g_LightDirection = mWaterParameters->g_LightDirection;
	cWB.g_FlowMapOffset1 = mWaterParameters->g_FlowMapOffset1;
	cWB.g_RefractionTint = mWaterParameters->g_RefractionTint;
	cWB.g_DullColor = mWaterParameters->g_DullColor;
	cWB.g_wpadding = XMFLOAT2(0, 0);
	cWB.g_FarPlane = fFarPlane;
	cWB.g_TexScale = mWaterParameters->g_TexScale;

	// Map the water parameter constants
	{
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		m_pD3DSystem->GetDeviceContext()->Map(m_pWaterBufferCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		{
			assert(mapped_res.pData);
			*(cbWaterBuffer*)mapped_res.pData = cWB;
		}
		m_pD3DSystem->GetDeviceContext()->Unmap(m_pWaterBufferCB, 0);
	}

	// Map the matrix constants
	{
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		m_pD3DSystem->GetDeviceContext()->Map(m_pMatrixBufferCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		{
			assert(mapped_res.pData);
			*(cbMatrixBuffer*)mapped_res.pData = cMB;
		}
		m_pD3DSystem->GetDeviceContext()->Unmap(m_pMatrixBufferCB, 0);
	}

	ID3D11Buffer* vs_cbs[2] = { m_pWaterBufferCB, m_pMatrixBufferCB };
	m_pD3DSystem->GetDeviceContext()->VSSetConstantBuffers(0, 2, vs_cbs);

	ID3D11Buffer* ps_cbs[1] = { m_pWaterBufferCB };
	m_pD3DSystem->GetDeviceContext()->PSSetConstantBuffers(0, 1, ps_cbs);

	ID3D11ShaderResourceView* ps_srvs[6] = { m_FlowMap, m_NoiseMap, m_WaveMap0, m_WaveMap1, m_ReflectionMap, m_RefractionMap };
	ID3D11SamplerState* ps_samp[2] = { m_pD3DSystem->Mirror(), m_pD3DSystem->Linear() };

	if (!m_Wireframe)
	{
		// Assign Texture

		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 2, ps_samp);

		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 6, ps_srvs);

		if (mUseFlowMap)
		{
			SwitchTo("WaterFlowPS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
		}
		else
		{
			if (mUseGBuffer)
			{
				SwitchTo("WaterGBufferNormalPS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
			}
			else
			{
				SwitchTo("WaterNormalPS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
			}
		}
	}
	else
	{
		if (mUseFlowMap)
		{
			SwitchTo("WaterFlowWireframePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
		}
		else
		{
			if (mUseGBuffer)
			{
				SwitchTo("WaterGBufferNormalWireframePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
			}
			else
			{
				SwitchTo("WaterNormalWireframePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
			}
		}
	}

	if (mUseFlowMap)
	{
		SwitchTo("WaterFlowVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
	}
	else
	{
		if (mUseGBuffer)
		{
			SwitchTo("WaterGBufferNormalVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
		}
		else
		{
			SwitchTo("WaterNormalVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
		}
	}

	SetInputLayout("TreeShader");

	SetVertexShader();
	SetPixelShader();

	//Perform Drawing
	RenderIndex11(indexCount);

	// Unbind
	if (!m_Wireframe)
	{
		ps_samp[0] = NULL;
		ps_samp[1] = NULL;
		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 2, ps_samp);

		for (int i = 0; i < 6; i++) ps_srvs[i] = NULL;
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 6, ps_srvs);
	}
}*/
//==============================================================================================================================