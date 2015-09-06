#include "MaterialGBufferShader.h"
#include "ConstantBuffer.h"
#include "CGlobal.h"
#include "LightShadingBuffers.h"
//==============================================================================================================================
MaterialGBufferShader::MaterialGBufferShader(D3D* d3d)
:	ZShadeSandboxShader::Shader(d3d)
{
	Initialize();
}
//==============================================================================================================================
MaterialGBufferShader::MaterialGBufferShader(const MaterialGBufferShader& other)
:	ZShadeSandboxShader::Shader(other)
{
}
//==============================================================================================================================
MaterialGBufferShader::~MaterialGBufferShader()
{
}
//==============================================================================================================================
bool MaterialGBufferShader::Initialize()
{
	ConstantBuffer<ZShadeSandboxLighting::cbMaterialShadingBuffer> packCB(m_pD3DSystem);
	packCB.Initialize(PAD16(sizeof(ZShadeSandboxLighting::cbMaterialShadingBuffer)));
	m_pPackCB = packCB.Buffer();
	
	ConstantBuffer<cbMatrixBuffer> matrixCB(m_pD3DSystem);
	matrixCB.Initialize(PAD16(sizeof(cbMatrixBuffer)));
	m_pMatrixCB = matrixCB.Buffer();
	
	ClearInputLayout();
	LoadPixelShader("MaterialGBufferPS");
	LoadPixelShader("MaterialGBufferWireframePS");
	
	SetInputLayoutDesc("MaterialGBufferShader", ZShadeSandboxMesh::VertexLayout::mesh_layout_pos_normal_tex_instance_rot_scl, 6);
	LoadVertexShader("MaterialGBufferVS");
	AssignVertexShaderLayout("MaterialGBufferShader");
	
	//SetInputLayoutDesc("MaterialGBufferShaderInstance", ZShadeSandboxMesh::VertexLayout::mesh_layout_pos_normal_tex_instance, 4);
	//LoadVertexShader("MaterialGBufferInstanceVS");
	//AssignVertexShaderLayout("MaterialGBufferShaderInstance");

	return true;
}
//==============================================================================================================================
void MaterialGBufferShader::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
}
//==============================================================================================================================
bool MaterialGBufferShader::Render11
(	int indexCount
,	int instanceCount
,	ZShadeSandboxMesh::MeshRenderParameters mrp
,	ZShadeSandboxLighting::ShaderMaterial* material
)
{
	if (material == 0) return false;
	
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

	material->BuildMaterialConstantBuffer(m_pPackCB, mrp.camera->Position(), mrp.clipplane);

	mrp.camera->BuildCameraConstantBuffer(m_pD3DSystem, m_pMatrixCB, mrp.light, mrp.world, mrp.reflection);
	
	ID3D11Buffer* vs_cbs[1] = { m_pMatrixCB };
	m_pD3DSystem->GetDeviceContext()->VSSetConstantBuffers(1, 1, vs_cbs);
	
	ID3D11Buffer* ps_cbs[1] = { m_pPackCB };
	m_pD3DSystem->GetDeviceContext()->PSSetConstantBuffers(0, 1, ps_cbs);
	
	// Assign Texture
	ID3D11ShaderResourceView* ps_srvs[11] = { diffuseArrayTexture, diffuseTexture, ambientTexture, specularTexture, emissiveTexture, normalMapTexture, blendMapTexture, detailMapTexture, alphaMapTexture, shadowMapTexture, ssaoTexture };
	ID3D11SamplerState* ps_samp[2] = { m_pD3DSystem->Point(), m_pD3DSystem->Linear() };
	
	ID3D11ShaderResourceView* vs_srvs[1] = { displacementMapTexture };
	
	if (!m_Wireframe)
	{
		m_pD3DSystem->GetDeviceContext()->VSSetSamplers(0, 2, ps_samp);
		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 2, ps_samp);
		
		m_pD3DSystem->GetDeviceContext()->VSSetShaderResources(11, 1, vs_srvs);
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 11, ps_srvs);
		
		SwitchTo("MaterialGBufferPS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	}
	else
	{
		SwitchTo("MaterialGBufferWireframePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	}

	if (mrp.useInstancing)
	{
		SetInputLayout("MaterialGBufferShaderInstance");
		SwitchTo("MaterialGBufferInstanceVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
	}
	else
	{
		SetInputLayout("MaterialGBufferShader");
		SwitchTo("MaterialGBufferVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
	}

	SetVertexShader();
	SetPixelShader();
	
	//Perform Drawing
	if (mrp.useInstancing)
	{
		RenderIndexInstanced11(indexCount, instanceCount);
	}
	else
	{
		RenderIndex11(indexCount);
	}

	// Unbind
	if (!m_Wireframe)
	{
		ps_samp[0] = NULL;
		ps_samp[1] = NULL;
		m_pD3DSystem->GetDeviceContext()->VSSetSamplers(0, 2, ps_samp);
		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 2, ps_samp);
		
		vs_srvs[0] = NULL;
		m_pD3DSystem->GetDeviceContext()->VSSetShaderResources(11, 1, vs_srvs);
		
		for (int i = 0; i < 11; i++) ps_srvs[i] = NULL;
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 11, ps_srvs);
	}

	return true;
}
//==============================================================================================================================
bool MaterialGBufferShader::Render11
(	int startIndex
,	int indexCount
,	int instanceCount
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
	
	material->BuildMaterialConstantBuffer(m_pPackCB, mrp.camera->Position(), mrp.clipplane);

	mrp.camera->BuildCameraConstantBuffer(m_pD3DSystem, m_pMatrixCB, mrp.light, mrp.world, mrp.reflection);
	
	ID3D11Buffer* vs_cbs[1] = { m_pMatrixCB };
	m_pD3DSystem->GetDeviceContext()->VSSetConstantBuffers(1, 1, vs_cbs);
	
	ID3D11Buffer* ps_cbs[1] = { m_pPackCB };
	m_pD3DSystem->GetDeviceContext()->PSSetConstantBuffers(0, 1, ps_cbs);
	
	// Assign Texture
	ID3D11ShaderResourceView* ps_srvs[11] = { diffuseArrayTexture, diffuseTexture, ambientTexture, specularTexture, emissiveTexture, normalMapTexture, blendMapTexture, detailMapTexture, alphaMapTexture, shadowMapTexture, ssaoTexture };
	ID3D11SamplerState* ps_samp[2] = { m_pD3DSystem->Point(), m_pD3DSystem->Linear() };
	
	ID3D11ShaderResourceView* vs_srvs[1] = { displacementMapTexture };
	
	if (!m_Wireframe)
	{
		m_pD3DSystem->GetDeviceContext()->VSSetSamplers(0, 2, ps_samp);
		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 2, ps_samp);
		
		m_pD3DSystem->GetDeviceContext()->VSSetShaderResources(11, 1, vs_srvs);
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 11, ps_srvs);
		
		SwitchTo("MaterialGBufferPS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	}
	else
	{
		SwitchTo("MaterialGBufferWireframePS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	}

	if (mrp.useInstancing)
	{
		SetInputLayout("MaterialGBufferShaderInstance");
		SwitchTo("MaterialGBufferInstanceVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
	}
	else
	{
		SetInputLayout("MaterialGBufferShader");
		SwitchTo("MaterialGBufferVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
	}

	SetVertexShader();
	SetPixelShader();
	
	//Perform Drawing
	if (mrp.useInstancing)
	{
		RenderIndexInstanced11(startIndex, indexCount, instanceCount);
	}
	else
	{
		RenderIndex11(startIndex, indexCount);
	}

	// Unbind
	if (!m_Wireframe)
	{
		ps_samp[0] = NULL;
		ps_samp[1] = NULL;
		m_pD3DSystem->GetDeviceContext()->VSSetSamplers(0, 2, ps_samp);
		m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 2, ps_samp);
		
		vs_srvs[0] = NULL;
		m_pD3DSystem->GetDeviceContext()->VSSetShaderResources(11, 1, vs_srvs);
		
		for (int i = 0; i < 11; i++) ps_srvs[i] = NULL;
		m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 11, ps_srvs);
	}

	return true;
}
//==============================================================================================================================