#include "OceanSimulationShader.h"
#include "ShaderCompiler.h"
#include "Shader.h"
#include "CGlobal.h"
#include "StructuredBuffer.h"
#include "RenderTarget2D.h"
#include "ConstantBuffer.h"
using ZShadeSandboxShader::OceanSimulationShader;
//==============================================================================================================================
//==============================================================================================================================

// Disable warning "conditional expression is constant"
#pragma warning(disable:4127)

// Already defined in ZMath.h
//#define HALF_SQRT_2	0.7071068f
//#define GRAV_ACCEL	981.0f	// The acceleration of gravity, cm/s^2
//#define BLOCK_SIZE_X 16
//#define BLOCK_SIZE_Y 16

//==============================================================================================================================
// Generating gaussian random number with mean 0 and standard deviation 1.
float Gauss()
{
	float u1 = rand() / (float)RAND_MAX;
	float u2 = rand() / (float)RAND_MAX;
	
	if (u1 < 1e-6f)
	{
		u1 = 1e-6f;
	}
	
	return sqrtf(-2 * logf(u1)) * cosf(2 * PI * u2);
}
//==============================================================================================================================
// Phillips Spectrum
// K: normalized wave vector, W: wind direction, v: wind velocity, a: amplitude constant
float Phillips(XMFLOAT2 K, XMFLOAT2 W, float v, float a, float dir_depend)
{
	// largest possible wave from constant wind of velocity v
	float l = v * v / GRAV_ACCEL;
	// damp out waves with very small length w << l
	float w = l / 1000;
	
	float Ksqr = K.x * K.x + K.y * K.y;
	float Kcos = K.x * W.x + K.y * W.y;
	float phillips = a * expf(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);
	
	// filter out waves moving opposite to wind
	if (Kcos < 0)
	{
		phillips *= dir_depend;
	}
	
	// damp out waves with very small length w << l
	return phillips * expf(-Ksqr * w * w);
}
//==============================================================================================================================
void createBufferAndUAV(ID3D11Device* pd3dDevice, void* data, UINT byte_width, UINT byte_stride,
						ID3D11Buffer** ppBuffer, ID3D11UnorderedAccessView** ppUAV, ID3D11ShaderResourceView** ppSRV)
{
	// Create buffer
	D3D11_BUFFER_DESC buf_desc;
	buf_desc.ByteWidth = byte_width;
    buf_desc.Usage = D3D11_USAGE_DEFAULT;
    buf_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    buf_desc.CPUAccessFlags = 0;
    buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    buf_desc.StructureByteStride = byte_stride;

	D3D11_SUBRESOURCE_DATA init_data = {data, 0, 0};

	pd3dDevice->CreateBuffer(&buf_desc, data != NULL ? &init_data : NULL, ppBuffer);
	assert(*ppBuffer);

	// Create undordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Buffer.NumElements = byte_width / byte_stride;
	uav_desc.Buffer.Flags = 0;

	pd3dDevice->CreateUnorderedAccessView(*ppBuffer, &uav_desc, ppUAV);
	assert(*ppUAV);

	// Create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srv_desc.Buffer.FirstElement = 0;
	srv_desc.Buffer.NumElements = byte_width / byte_stride;

	pd3dDevice->CreateShaderResourceView(*ppBuffer, &srv_desc, ppSRV);
	assert(*ppSRV);
}
//==============================================================================================================================
void createTextureAndViews(ID3D11Device* pd3dDevice, UINT width, UINT height, DXGI_FORMAT format,
						   ID3D11Texture2D** ppTex, ID3D11ShaderResourceView** ppSRV, ID3D11RenderTargetView** ppRTV)
{
	// Create 2D texture
	D3D11_TEXTURE2D_DESC tex_desc;
	tex_desc.Width = width;
	tex_desc.Height = height;
	tex_desc.MipLevels = 0;
	tex_desc.ArraySize = 1;
	tex_desc.Format = format;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	pd3dDevice->CreateTexture2D(&tex_desc, NULL, ppTex);
	assert(*ppTex);

	// Create shader resource view
	(*ppTex)->GetDesc(&tex_desc);
	if (ppSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		srv_desc.Format = format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = tex_desc.MipLevels;
		srv_desc.Texture2D.MostDetailedMip = 0;

		pd3dDevice->CreateShaderResourceView(*ppTex, &srv_desc, ppSRV);
		assert(*ppSRV);
	}

	// Create render target view
	if (ppRTV)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		rtv_desc.Format = format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtv_desc.Texture2D.MipSlice = 0;

		pd3dDevice->CreateRenderTargetView(*ppTex, &rtv_desc, ppRTV);
		assert(*ppRTV);
	}
}
//==============================================================================================================================
OceanSimulationShader::OceanSimulationShader(D3D* d3d, OceanParameters& params)
:	ZShadeSandboxShader::Shader(d3d)
{
	m_param = params;
	
	// If the device becomes invalid at some point, delete current instance and generate a new one.
	//m_pd3dDevice = pd3dDevice;
	//assert(m_pd3dDevice);
	
	//m_pd3dDevice->GetImmediateContext(&m_pd3dImmediateContext);
	//assert(m_pd3dImmediateContext);
	
	// The perlin texture name should be in ocean params so the editor can use any texture
	m_pPerlinSRV = TextureManager::Instance()->GetTexture("Textures\\perlin_noise.dds");
	assert(m_pPerlinSRV);
	
	// Height map H(0)
	int height_map_size = (params.g_MapDimension + 4) * (params.g_MapDimension + 1);
	XMFLOAT2* h0_data = new XMFLOAT2[height_map_size * sizeof(XMFLOAT2)];
	float* omega_data = new float[height_map_size * sizeof(float)];
	initHeightMap(params, h0_data, omega_data);
	
	int hmap_dim = params.g_MapDimension;
	int input_full_size = (hmap_dim + 4) * (hmap_dim + 1);
	// This value should be (hmap_dim / 2 + 1) * hmap_dim, but we use full sized buffer here for simplicity.
	int input_half_size = hmap_dim * hmap_dim;
	int output_size = hmap_dim * hmap_dim;
	
	// For filling the buffer with zeroes.
	char* zero_data = new char[3 * output_size * sizeof(float) * 2];
	memset(zero_data, 0, 3 * output_size * sizeof(float) * 2);
	
	// RW buffer allocations
	// H0
	UINT float2_stride = 2 * sizeof(float);
	//createBufferAndUAV(m_pD3DSystem->GetDevice11(), h0_data, input_full_size * float2_stride, float2_stride, &m_pBuffer_Float2_H0, &m_pUAV_H0, &m_pSRV_H0);
	{
		UINT stride = float2_stride;
		UINT byteWidth = input_full_size * float2_stride;
		UINT numElements = byteWidth;
		UINT srvElementCount = byteWidth / stride;
		bool useAsUAV = true;
		UINT uavElementCount = byteWidth / stride;
		bool appendConsume = false;
		bool useAsDrawIndirect = false;
		
		h0SB.SetD3D(m_pD3DSystem);
		
		h0SB.Initialize(
			stride,
			byteWidth,
			numElements,
			srvElementCount,
			useAsUAV,
			uavElementCount,
			appendConsume,
			useAsDrawIndirect,
			h0_data
		);
		
		m_pBuffer_Float2_H0 = h0SB.GetBuffer();
		m_pUAV_H0 = h0SB.UAView;
		m_pSRV_H0 = h0SB.SRView;
	}
	
	// Notice: The following 3 buffers should be half sized buffer because of conjugate symmetric input. But
	// we use full sized buffers due to the CS4.0 restriction.
	
	// Put H(t), Dx(t) and Dy(t) into one buffer because CS4.0 allows only 1 UAV at a time
	//createBufferAndUAV(m_pD3DSystem->GetDevice11(), zero_data, 3 * input_half_size * float2_stride, float2_stride, &m_pBuffer_Float2_Ht, &m_pUAV_Ht, &m_pSRV_Ht);
	{
		UINT stride = float2_stride;
		UINT byteWidth = 3 * input_half_size * float2_stride;
		UINT numElements = byteWidth;
		UINT srvElementCount = byteWidth / stride;
		bool useAsUAV = true;
		UINT uavElementCount = byteWidth / stride;
		bool appendConsume = false;
		bool useAsDrawIndirect = false;
		
		HtSB.SetD3D(m_pD3DSystem);

		HtSB.Initialize(
			stride,
			byteWidth,
			numElements,
			srvElementCount,
			useAsUAV,
			uavElementCount,
			appendConsume,
			useAsDrawIndirect,
			zero_data
		);
		
		m_pBuffer_Float2_Ht = HtSB.GetBuffer();
		m_pUAV_Ht = HtSB.UAView;
		m_pSRV_Ht = HtSB.SRView;
	}
	
	// omega
	//createBufferAndUAV(m_pD3DSystem->GetDevice11(), omega_data, input_full_size * sizeof(float), sizeof(float), &m_pBuffer_Float_Omega, &m_pUAV_Omega, &m_pSRV_Omega);
	{
		UINT stride = sizeof(float);
		UINT byteWidth = input_full_size * sizeof(float);
		UINT numElements = byteWidth;
		UINT srvElementCount = byteWidth / stride;
		bool useAsUAV = true;
		UINT uavElementCount = byteWidth / stride;
		bool appendConsume = false;
		bool useAsDrawIndirect = false;
		
		omegaSB.SetD3D(m_pD3DSystem);

		omegaSB.Initialize(
			stride,
			byteWidth,
			numElements,
			srvElementCount,
			useAsUAV,
			uavElementCount,
			appendConsume,
			useAsDrawIndirect,
			omega_data
		);
		
		m_pBuffer_Float_Omega = omegaSB.GetBuffer();
		m_pUAV_Omega = omegaSB.UAView;
		m_pSRV_Omega = omegaSB.SRView;
	}
	
	// Notice: The following 3 should be real number data. But here we use the complex numbers and C2C FFT
	// due to the CS4.0 restriction.
	// Put Dz, Dx and Dy into one buffer because CS4.0 allows only 1 UAV at a time
	//createBufferAndUAV(m_pD3DSystem->GetDevice11(), zero_data, 3 * output_size * float2_stride, float2_stride, &m_pBuffer_Float_Dxyz, &m_pUAV_Dxyz, &m_pSRV_Dxyz);
	{
		UINT stride = float2_stride;
		UINT byteWidth = 3 * output_size * float2_stride;
		UINT numElements = byteWidth;
		UINT srvElementCount = byteWidth / stride;
		bool useAsUAV = true;
		UINT uavElementCount = byteWidth / stride;
		bool appendConsume = false;
		bool useAsDrawIndirect = false;
		
		xyzSB.SetD3D(m_pD3DSystem);

		xyzSB.Initialize(
			stride,
			byteWidth,
			numElements,
			srvElementCount,
			useAsUAV,
			uavElementCount,
			appendConsume,
			useAsDrawIndirect,
			zero_data
		);
		
		m_pBuffer_Float_Dxyz = xyzSB.GetBuffer();
		m_pUAV_Dxyz = xyzSB.UAView;
		m_pSRV_Dxyz = xyzSB.SRView;
	}
	
	SAFE_DELETE_ARRAY(zero_data);
	SAFE_DELETE_ARRAY(h0_data);
	SAFE_DELETE_ARRAY(omega_data);

	// D3D11 Textures
	//createTextureAndViews(m_pD3DSystem->GetDevice11(), hmap_dim, hmap_dim, DXGI_FORMAT_R32G32B32A32_FLOAT, &m_pDisplacementMap, &m_pDisplacementSRV, &m_pDisplacementRTV);
	//createTextureAndViews(m_pD3DSystem->GetDevice11(), hmap_dim, hmap_dim, DXGI_FORMAT_R16G16B16A16_FLOAT, &m_pGradientMap, &m_pGradientSRV, &m_pGradientRTV);
	
	int textureWidth = hmap_dim;
	int textureHeight = hmap_dim;
	DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	UINT multiSamples = 1;
	UINT msQuality = 0;
	UINT numMipLevels = 0;
	bool autoGenMipMaps = true;
	bool createUAV = false;
	bool srvAsCube = false;
	
	// Create the displacement map render target
	{
		//RenderTarget2D rt(m_pD3DSystem);
		displacementRT.SetD3D(m_pD3DSystem);

		displacementRT.Initialize(
			textureWidth,
			textureHeight,
			format,
			multiSamples,
			msQuality,
			numMipLevels,
			autoGenMipMaps,
			createUAV,
			srvAsCube
		);
		
		m_pDisplacementMap = displacementRT.RTVTexture;
		m_pDisplacementSRV = displacementRT.SRView;
		m_pDisplacementRTV = displacementRT.RTView;
	}
	
	// Create the gradient map render target
	{
		format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		
		//RenderTarget2D rt(m_pD3DSystem);
		gradientRT.SetD3D(m_pD3DSystem);

		gradientRT.Initialize(
			textureWidth,
			textureHeight,
			format,
			multiSamples,
			msQuality,
			numMipLevels,
			autoGenMipMaps,
			createUAV,
			srvAsCube
		);
		
		m_pGradientMap = gradientRT.RTVTexture;
		m_pGradientSRV = gradientRT.SRView;
		m_pGradientRTV = gradientRT.RTView;
	}
	
	/*
	// Samplers
	D3D11_SAMPLER_DESC sam_desc;
	sam_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sam_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sam_desc.MipLODBias = 0; 
	sam_desc.MaxAnisotropy = 1; 
	sam_desc.ComparisonFunc = D3D11_COMPARISON_NEVER; 
	sam_desc.BorderColor[0] = 1.0f;
	sam_desc.BorderColor[1] = 1.0f;
	sam_desc.BorderColor[2] = 1.0f;
	sam_desc.BorderColor[3] = 1.0f;
	sam_desc.MinLOD = -FLT_MAX;
	sam_desc.MaxLOD = FLT_MAX;
	m_pD3DSystem->GetDevice11()->CreateSamplerState(&sam_desc, &m_pPointSamplerState);
	assert(m_pPointSamplerState);
	
	// Compute shaders
	ID3DBlob* pBlobUpdateSpectrumCS = NULL;
	
	//get the shader blobs from compiler
	pBlobUpdateSpectrumCS = ZShadeSandboxShader::ShaderCompiler::GetShaderByName("UpdateSpectrumCS")->mBlobData;
	assert(pBlobUpdateSpectrumCS);
	
	m_pD3DSystem->GetDevice11()->CreateComputeShader(pBlobUpdateSpectrumCS->GetBufferPointer(), pBlobUpdateSpectrumCS->GetBufferSize(), NULL, &m_pUpdateSpectrumCS);
	assert(m_pUpdateSpectrumCS);
	
	SAFE_RELEASE(pBlobUpdateSpectrumCS);
	
	// Vertex & pixel shaders
	ID3DBlob* pBlobQuadVS = NULL;
	ID3DBlob* pBlobUpdateDisplacementPS = NULL;
	ID3DBlob* pBlobGenGradientFoldingPS = NULL;
	
	pBlobQuadVS = ZShadeSandboxShader::ShaderCompiler::GetShaderByName("QuadVS")->mBlobData;
	pBlobUpdateDisplacementPS = ZShadeSandboxShader::ShaderCompiler::GetShaderByName("UpdateDisplacementPS")->mBlobData;
	pBlobGenGradientFoldingPS = ZShadeSandboxShader::ShaderCompiler::GetShaderByName("GenGradientFoldingPS")->mBlobData;
	
	assert(pBlobQuadVS);
	assert(pBlobUpdateDisplacementPS);
	assert(pBlobGenGradientFoldingPS);
	
	m_pD3DSystem->GetDevice11()->CreateVertexShader(pBlobQuadVS->GetBufferPointer(), pBlobQuadVS->GetBufferSize(), NULL, &m_pQuadVS);
	m_pD3DSystem->GetDevice11()->CreatePixelShader(pBlobUpdateDisplacementPS->GetBufferPointer(), pBlobUpdateDisplacementPS->GetBufferSize(), NULL, &m_pUpdateDisplacementPS);
	m_pD3DSystem->GetDevice11()->CreatePixelShader(pBlobGenGradientFoldingPS->GetBufferPointer(), pBlobGenGradientFoldingPS->GetBufferSize(), NULL, &m_pGenGradientFoldingPS);
	assert(m_pQuadVS);
	assert(m_pUpdateDisplacementPS);
	assert(m_pGenGradientFoldingPS);
	SAFE_RELEASE(pBlobUpdateDisplacementPS);
	SAFE_RELEASE(pBlobGenGradientFoldingPS);
	
	// Input layout
	D3D11_INPUT_ELEMENT_DESC quad_layout_desc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	m_pD3DSystem->GetDevice11()->CreateInputLayout(quad_layout_desc, 1, pBlobQuadVS->GetBufferPointer(), pBlobQuadVS->GetBufferSize(), &m_pQuadLayout);
	assert(m_pQuadLayout);
	
	SAFE_RELEASE(pBlobQuadVS);
	*/
	
	ClearInputLayout();
	SetInputLayoutDesc("OceanShader", ZShadeSandboxMesh::VertexLayout::mesh_layout_pos, 1);
	LoadVertexShader("QuadVS");
	LoadComputeShader("UpdateSpectrumCS");
	LoadPixelShader("UpdateDisplacementPS");
	LoadPixelShader("GenGradientFoldingPS");
	AssignVertexShaderLayout("OceanShader");
	
	// Quad vertex buffer
	D3D11_BUFFER_DESC vb_desc;
	vb_desc.ByteWidth = 4 * sizeof(XMFLOAT4);
	vb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb_desc.CPUAccessFlags = 0;
	vb_desc.MiscFlags = 0;
	
	float quad_verts[] =
	{
		-1, -1, 0, 1,
		-1,  1, 0, 1,
		 1, -1, 0, 1,
		 1,  1, 0, 1,
	};
	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = &quad_verts[0];
	init_data.SysMemPitch = 0;
	init_data.SysMemSlicePitch = 0;
	
	m_pD3DSystem->GetDevice11()->CreateBuffer(&vb_desc, &init_data, &m_pQuadVB);
	assert(m_pQuadVB);

	// Constant buffers
	UINT actual_dim = m_param.g_MapDimension;
	UINT input_width = actual_dim + 4;
	// We use full sized data here. The value "output_width" should be actual_dim/2+1 though.
	UINT output_width = actual_dim;
	UINT output_height = actual_dim;
	UINT dtx_offset = actual_dim * actual_dim;
	UINT dty_offset = actual_dim * actual_dim * 2;
	UINT immutable_consts[] = {actual_dim, input_width, output_width, output_height, dtx_offset, dty_offset};
	D3D11_SUBRESOURCE_DATA init_cb0 = {&immutable_consts[0], 0, 0};
	
	D3D11_BUFFER_DESC cb_desc;
	cb_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = 0;
	cb_desc.MiscFlags = 0;    
	cb_desc.ByteWidth = PAD16(sizeof(immutable_consts));
	m_pD3DSystem->GetDevice11()->CreateBuffer(&cb_desc, &init_cb0, &m_pImmutableCB);
	assert(m_pImmutableCB);
	
	ID3D11Buffer* cbs[1] = {m_pImmutableCB};
	m_pD3DSystem->GetDeviceContext()->CSSetConstantBuffers(0, 1, cbs);
	m_pD3DSystem->GetDeviceContext()->PSSetConstantBuffers(0, 1, cbs);
	
	cb_desc.Usage = D3D11_USAGE_DYNAMIC;
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb_desc.MiscFlags = 0;    
	cb_desc.ByteWidth = PAD16(sizeof(float) * 3);
	m_pD3DSystem->GetDevice11()->CreateBuffer(&cb_desc, NULL, &m_pPerFrameCB);
	assert(m_pPerFrameCB);
	
	/*ConstantBuffer<UINT> immutableCB(m_pD3DSystem);
	immutableCB.Initialize(PAD16(sizeof(immutable_consts)), &init_cb0, true);
	m_pImmutableCB = immutableCB.Buffer();
	
	ConstantBuffer<float> perFrameCB(m_pD3DSystem);
	perFrameCB.Initialize(PAD16(sizeof(float) * 3));
	m_pPerFrameCB = perFrameCB.Buffer();
	
	ID3D11Buffer* cbs[1] = {m_pImmutableCB};
	m_pD3DSystem->GetDeviceContext()->CSSetConstantBuffers(0, 1, cbs);
	m_pD3DSystem->GetDeviceContext()->PSSetConstantBuffers(0, 1, cbs);*/
	
	// FFT
	fft512x512_create_plan(&m_fft_plan, m_pD3DSystem->GetDevice11(), 3);
	
//#ifdef CS_DEBUG_BUFFER
	D3D11_BUFFER_DESC buf_desc;
	buf_desc.ByteWidth = 3 * input_half_size * float2_stride;
	buf_desc.Usage = D3D11_USAGE_STAGING;
	buf_desc.BindFlags = 0;
	buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	buf_desc.StructureByteStride = float2_stride;
	
	m_pD3DSystem->GetDevice11()->CreateBuffer(&buf_desc, NULL, &m_pDebugBuffer);
	assert(m_pDebugBuffer);
	m_pD3DSystem->GetDevice11()->CreateBuffer(&buf_desc, NULL, &m_pDebugBuffer2);
	assert(m_pDebugBuffer2);
//#endif
}
//==============================================================================================================================
OceanSimulationShader::~OceanSimulationShader()
{
	fft512x512_destroy_plan(&m_fft_plan);

	SAFE_RELEASE(m_pBuffer_Float2_H0);
	SAFE_RELEASE(m_pBuffer_Float_Omega);
	SAFE_RELEASE(m_pBuffer_Float2_Ht);
	SAFE_RELEASE(m_pBuffer_Float_Dxyz);

	//SAFE_RELEASE(m_pPointSamplerState);

	SAFE_RELEASE(m_pQuadVB);

	SAFE_RELEASE(m_pUAV_H0);
	SAFE_RELEASE(m_pUAV_Omega);
	SAFE_RELEASE(m_pUAV_Ht);
	SAFE_RELEASE(m_pUAV_Dxyz);

	SAFE_RELEASE(m_pSRV_H0);
	SAFE_RELEASE(m_pSRV_Omega);
	SAFE_RELEASE(m_pSRV_Ht);
	SAFE_RELEASE(m_pSRV_Dxyz);

	SAFE_RELEASE(m_pDisplacementMap);
	SAFE_RELEASE(m_pDisplacementSRV);
	SAFE_RELEASE(m_pDisplacementRTV);

	SAFE_RELEASE(m_pGradientMap);
	SAFE_RELEASE(m_pGradientSRV);
	SAFE_RELEASE(m_pGradientRTV);

	//SAFE_RELEASE(m_pUpdateSpectrumCS);
	//SAFE_RELEASE(m_pQuadVS);
	//SAFE_RELEASE(m_pUpdateDisplacementPS);
	//SAFE_RELEASE(m_pGenGradientFoldingPS);

	//SAFE_RELEASE(m_pQuadLayout);

	SAFE_RELEASE(m_pImmutableCB);
	SAFE_RELEASE(m_pPerFrameCB);

	//SAFE_RELEASE(m_pd3dImmediateContext);

//#ifdef CS_DEBUG_BUFFER
	SAFE_RELEASE(m_pDebugBuffer);
	SAFE_RELEASE(m_pDebugBuffer2);
//#endif
}
//==============================================================================================================================
// Initialize the vector field.
// wlen_x: width of wave tile, in meters
// wlen_y: length of wave tile, in meters
void OceanSimulationShader::initHeightMap(OceanParameters& params, XMFLOAT2* out_h0, float* out_omega)
{
	int i, j;
	XMFLOAT2 K, Kn;

   XMFLOAT2 wind_dir = ZShadeSandboxMath::XMMath2(params.g_WindDir).Normalize();
   
   float a = params.g_WaveAmplitude * 1e-7f;	// It is too small. We must scale it for editing.
	float v = params.g_WindSpeed;
	float dir_depend = params.g_WindDependency;

	int height_map_dim = params.g_MapDimension;
   float patch_length = params.g_PatchLength;

	// initialize random generator.
	srand(0);

	for (i = 0; i <= height_map_dim; i++)
	{
		// K is wave-vector, range [-|DX/W, |DX/W], [-|DY/H, |DY/H]
		K.y = (-height_map_dim / 2.0f + i) * (2 * PI / patch_length);

		for (j = 0; j <= height_map_dim; j++)
		{
			K.x = (-height_map_dim / 2.0f + j) * (2 * PI / patch_length);

			float phil = (K.x == 0 && K.y == 0) ? 0 : sqrtf(Phillips(K, wind_dir, v, a, dir_depend));

			out_h0[i * (height_map_dim + 4) + j].x = float(phil * Gauss() * HALF_SQRT_2);
			out_h0[i * (height_map_dim + 4) + j].y = float(phil * Gauss() * HALF_SQRT_2);

			// The angular frequency is following the dispersion relation:
			//            out_omega^2 = g*k
			// The equation of Gerstner wave:
			//            x = x0 - K/k * A * sin(dot(K, x0) - sqrt(g * k) * t), x is a 2D vector.
			//            z = A * cos(dot(K, x0) - sqrt(g * k) * t)
			// Gerstner wave shows that a point on a simple sinusoid wave is doing a uniform circular
			// motion with the center (x0, y0, z0), radius A, and the circular plane is parallel to
			// vector K.
			out_omega[i * (height_map_dim + 4) + j] = sqrtf(GRAV_ACCEL * sqrtf(K.x * K.x + K.y * K.y));
		}
	}
}
//==============================================================================================================================
XMFLOAT3 OceanSimulationShader::ReadWave(int x, int z)
{
	//map this to appropriate postion in displacement map space
	
	int addr = m_param.g_MapDimension * z + x;
	
	UINT dtx_offset = m_param.g_MapDimension * m_param.g_MapDimension;
	UINT dty_offset = dtx_offset * 2;
	
	float dx = v[addr + dtx_offset].x;
	float dy = v[addr + dty_offset].x;
	float dz = v[addr].x;
	
	dz += h[addr];
	
	return XMFLOAT3(dx, dz, dy);
}
//==============================================================================================================================
void OceanSimulationShader::updateDisplacementMap(float time)
{
	// ---------------------------- H(0) -> H(t), D(x, t), D(y, t) --------------------------------
	// Compute shader
	//m_pD3DSystem->GetDeviceContext()->CSSetShader(m_pUpdateSpectrumCS, NULL, 0);
	SwitchTo("UpdateSpectrumCS", ZShadeSandboxShader::EShaderTypes::ST_COMPUTE);
	SetComputeShader();
	
	// Buffers
	ID3D11ShaderResourceView* cs0_srvs[2] = {m_pSRV_H0, m_pSRV_Omega};
	m_pD3DSystem->GetDeviceContext()->CSSetShaderResources(0, 2, cs0_srvs);
	
	ID3D11UnorderedAccessView* cs0_uavs[1] = {m_pUAV_Ht};
	m_pD3DSystem->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, cs0_uavs, (UINT*)(&cs0_uavs[0]));
	
	// Consts
	D3D11_MAPPED_SUBRESOURCE mapped_res;            
	m_pD3DSystem->GetDeviceContext()->Map(m_pPerFrameCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
	assert(mapped_res.pData);
	float* per_frame_data = (float*)mapped_res.pData;
	// g_Time
	per_frame_data[0] = time * m_param.g_TimeScale;
	// g_ChoppyScale
	per_frame_data[1] = m_param.g_ChoppyScale;
	// g_GridLen
	per_frame_data[2] = m_param.g_MapDimension / m_param.g_PatchLength;
	m_pD3DSystem->GetDeviceContext()->Unmap(m_pPerFrameCB, 0);
	
	ID3D11Buffer* cs_cbs[2] = {m_pImmutableCB, m_pPerFrameCB};
	m_pD3DSystem->GetDeviceContext()->CSSetConstantBuffers(0, 2, cs_cbs);
	
	// Run the CS
	UINT group_count_x = (m_param.g_MapDimension + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
	UINT group_count_y = (m_param.g_MapDimension + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;
	//m_pD3DSystem->GetDeviceContext()->Dispatch(group_count_x, group_count_y, 1);
	RenderCS11(group_count_x, group_count_y, 1);
	
	// Unbind resources for CS
	cs0_uavs[0] = NULL;
	m_pD3DSystem->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, cs0_uavs, (UINT*)(&cs0_uavs[0]));
	cs0_srvs[0] = NULL;
	cs0_srvs[1] = NULL;
	m_pD3DSystem->GetDeviceContext()->CSSetShaderResources(0, 2, cs0_srvs);
	
	// ------------------------------------ Perform FFT -------------------------------------------
	fft_512x512_c2c(&m_fft_plan, m_pUAV_Dxyz, m_pSRV_Dxyz, m_pSRV_Ht);
	
	// --------------------------------- Wrap Dx, Dy and Dz ---------------------------------------
	// Push RT
	ID3D11RenderTargetView* old_target;
	ID3D11DepthStencilView* old_depth;
	m_pD3DSystem->GetDeviceContext()->OMGetRenderTargets(1, &old_target, &old_depth); 
	D3D11_VIEWPORT old_viewport;
	UINT num_viewport = 1;
	m_pD3DSystem->GetDeviceContext()->RSGetViewports(&num_viewport, &old_viewport);
	
	D3D11_VIEWPORT new_vp = { 0, 0, (float)m_param.g_MapDimension, (float)m_param.g_MapDimension, 0.0f, 1.0f };
	m_pD3DSystem->GetDeviceContext()->RSSetViewports(1, &new_vp);
	
	// Set RT
	ID3D11RenderTargetView* rt_views[1] = {m_pDisplacementRTV};
	m_pD3DSystem->GetDeviceContext()->OMSetRenderTargets(1, rt_views, NULL);
	
	// VS & PS
	//m_pD3DSystem->GetDeviceContext()->VSSetShader(m_pQuadVS, NULL, 0);
	//m_pD3DSystem->GetDeviceContext()->PSSetShader(m_pUpdateDisplacementPS, NULL, 0);
	
	// VS & PS
	SwitchTo("QuadVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
	SwitchTo("UpdateDisplacementPS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	
	SetVertexShader();
	SetPixelShader();
	
	// Constants
	ID3D11Buffer* ps_cbs[2] = {m_pImmutableCB, m_pPerFrameCB};
	m_pD3DSystem->GetDeviceContext()->PSSetConstantBuffers(0, 2, ps_cbs);
	
	// Buffer resources
	ID3D11ShaderResourceView* ps_srvs[1] = {m_pSRV_Dxyz};
	m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);
	
	// IA setup
	ID3D11Buffer* vbs[1] = {m_pQuadVB};
	UINT strides[1] = {sizeof(XMFLOAT4)};
	UINT offsets[1] = {0};
	m_pD3DSystem->GetDeviceContext()->IASetVertexBuffers(0, 1, &vbs[0], &strides[0], &offsets[0]);
	
	SetInputLayout("OceanShader");
	//m_pD3DSystem->GetDeviceContext()->IASetInputLayout(m_pQuadLayout);
	m_pD3DSystem->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
	// Perform draw call
	//m_pD3DSystem->GetDeviceContext()->Draw(4, 0);
	RenderDraw11(4);
	
	// Unbind
	ps_srvs[0] = NULL;
	m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);
	
	// ----------------------------------- Generate Normal ----------------------------------------
	// Set RT
	rt_views[0] = m_pGradientRTV;
	m_pD3DSystem->GetDeviceContext()->OMSetRenderTargets(1, rt_views, NULL);
	
	// VS & PS
	//m_pD3DSystem->GetDeviceContext()->VSSetShader(m_pQuadVS, NULL, 0);
	//m_pD3DSystem->GetDeviceContext()->PSSetShader(m_pGenGradientFoldingPS, NULL, 0);
	
	// VS & PS
	SwitchTo("QuadVS", ZShadeSandboxShader::EShaderTypes::ST_VERTEX);
	SwitchTo("GenGradientFoldingPS", ZShadeSandboxShader::EShaderTypes::ST_PIXEL);
	
	SetVertexShader();
	SetPixelShader();
	
	// Texture resource and sampler
	ps_srvs[0] = m_pDisplacementSRV;
	m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);
	
	ID3D11SamplerState* samplers[1] = { m_pD3DSystem->Point() };//m_pPointSamplerState};
	m_pD3DSystem->GetDeviceContext()->PSSetSamplers(0, 1, &samplers[0]);
	
	// Perform draw call
	//m_pD3DSystem->GetDeviceContext()->Draw(4, 0);
	RenderDraw11(4);
	
	// Unbind
	ps_srvs[0] = NULL;
	m_pD3DSystem->GetDeviceContext()->PSSetShaderResources(0, 1, ps_srvs);
	
	// Pop RT
	m_pD3DSystem->GetDeviceContext()->RSSetViewports(1, &old_viewport);
	m_pD3DSystem->GetDeviceContext()->OMSetRenderTargets(1, &old_target, old_depth);
	SAFE_RELEASE(old_target);
	SAFE_RELEASE(old_depth);
	
	m_pD3DSystem->GetDeviceContext()->GenerateMips(m_pGradientSRV);
	
	// only need to grab the initial height once
	static bool once = true;
	
	// This drops FPS by 10
	if (true)
	{
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		D3D11_MAPPED_SUBRESOURCE mapped_res2;
		
		m_pD3DSystem->GetDeviceContext()->CopyResource(m_pDebugBuffer, m_pBuffer_Float_Dxyz);
		m_pD3DSystem->GetDeviceContext()->Map(m_pDebugBuffer, 0, D3D11_MAP_READ, 0, &mapped_res);
		
		if (once)
		{      
			m_pD3DSystem->GetDeviceContext()->CopyResource(m_pDebugBuffer2, m_pBuffer_Float2_H0);
			m_pD3DSystem->GetDeviceContext()->Map(m_pDebugBuffer2, 0, D3D11_MAP_READ, 0, &mapped_res2);
		}
		
		v = (XMFLOAT2*)mapped_res.pData;
		
		if (once)
		{
			h = (float*)mapped_res2.pData;
		}
		
		m_pD3DSystem->GetDeviceContext()->Unmap(m_pDebugBuffer, 0);
		
		if (once)
		{
			once = false;
			m_pD3DSystem->GetDeviceContext()->Unmap(m_pDebugBuffer2, 0);
		}
	}
	
	// Define CS_DEBUG_BUFFER to enable writing a buffer into a file.
/*#ifdef CS_DEBUG_BUFFER
    {
		m_pD3DSystem->GetDeviceContext()->CopyResource(m_pDebugBuffer, m_pBuffer_Float_Dxyz);
        D3D11_MAPPED_SUBRESOURCE mapped_res;
        m_pD3DSystem->GetDeviceContext()->Map(m_pDebugBuffer, 0, D3D11_MAP_READ, 0, &mapped_res);
        
		// set a break point below, and drag MappedResource.pData into in your Watch window
		// and cast it as (float*)

		// Write to disk
		XMFLOAT2* v = (XMFLOAT2*)mapped_res.pData;

		FILE* fp = fopen(".\\tmp\\Ht_raw.dat", "wb");
		fwrite(v, 512*512*sizeof(float)*2*3, 1, fp);
		fclose(fp);

		m_pD3DSystem->GetDeviceContext()->Unmap(m_pDebugBuffer, 0);
    }
#endif
*/
}
//==============================================================================================================================
ID3D11ShaderResourceView* OceanSimulationShader::getD3D11DisplacementMap()
{
	return m_pDisplacementSRV;
}
//==============================================================================================================================
ID3D11ShaderResourceView* OceanSimulationShader::getD3D11GradientMap()
{
	return m_pGradientSRV;
}
//==============================================================================================================================
ID3D11ShaderResourceView* OceanSimulationShader::getD3D11PerlinMap()
{
	return m_pPerlinSRV;
}
//==============================================================================================================================
const OceanParameters& OceanSimulationShader::getParameters()
{
	return m_param;
}
//==============================================================================================================================