//==============================================================================
// Material GBuffer
//
//==============================================================================
// History
//
// -Created on 6/7/2015 by Dustin Watson
//==============================================================================
#include "../../../Basic/Matrix.hlsl"

//======================================================================================================

//
// Constants
//

cbuffer cbShadingBuffer : register(b0)
{
	float3	g_EyePosW;
	float	g_Alpha;
	float4	g_ClipPlane;
	float4	g_MaterialDiffuseColor;
	float4	g_MaterialAmbientColor;
	float4	g_MaterialSpecularColor;
	float4	g_MaterialEmissiveColor;
	float	g_Emissivity;
	float	g_Reflectivity;
	float	g_Transmissivity;
	float	g_RefractionIndex;
	float3	g_AlphaToCoverageValue;
	float	g_DetailBrightness;
	float3	g_TransmissionFilter;
	float	g_MaterialSpecularPower;
	float	g_MaterialSpecularIntensity;
	int		g_IlluminationModel;
	int	   	g_FlipTextureH;
	int	   	g_FlipTextureV;
	int		g_UsingDiffuseArrayTexture;
	int	   	g_UsingDiffuseTexture;
	int	   	g_UsingAmbientTexture;
	int	   	g_UsingSpecularTexture;
	int	   	g_UsingEmissiveTexture;
	int	   	g_UsingNormalMapTexture;
	int	   	g_UsingBlendMapTexture;
	int	   	g_UsingDetailMapTexture;
	int		g_UsingAlphaMapTexture;
	int		g_UsingTransparency;
	int		g_UsingShadowMap;
	int		g_UsingSSAOMap;
	int		g_UsingDisplacementMap;
	float	g_FarPlane;
	int		g_SpecularToggle;
	int		g_EnableLighting;
	float2	matpadding;
	int		g_UsingInstancing;
	float	g_BlendAmount;
};

cbuffer cbMatrixBuffer : register(b1)
{
	matrix g_World;
	matrix g_View;
	matrix g_Proj;
};

//======================================================================================================

//
// Samplers and textures
//

Texture2DArray g_MaterialDiffuseArrayTexture 	: register(t0);
Texture2D g_MaterialDiffuseTexture 				: register(t1);
Texture2D g_MaterialAmbientTexture 				: register(t2);
Texture2D g_MaterialSpecularTexture 			: register(t3);
Texture2D g_MaterialEmissiveTexture 			: register(t4);
Texture2D g_MaterialNormalMapTexture 			: register(t5);
Texture2D g_MaterialBlendMapTexture 			: register(t6);
Texture2D g_MaterialDetailMapTexture 			: register(t7);
Texture2D g_MaterialAlphaMapTexture 			: register(t8);
Texture2D g_ShadowMap							: register(t9);
Texture2D g_SSAOMap								: register(t10);
Texture2D g_DisplacementMap						: register(t11);

SamplerState g_PointSampler		: register(s0);
SamplerState g_LinearSampler	: register(s1);

//======================================================================================================

//
// Structs
//

struct VertexInput
{
	float3 position		: POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD0;
	float3 instancePos	: INSTANCEPOS;
	float3 instanceRot	: INSTANCEROT;
	float3 instanceScl	: INSTANCESCL;
	uint instanceID		: SV_InstanceID;
};

struct PixelInput
{
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float2 uv			: TEXCOORD0;
	float2 depth		: TEXCOORD1;
	float instanceIndex	: TEXCOORD2;
};

struct PixelOutput
{
	float4 colors 	: SV_Target0;
	float4 normals 	: SV_Target1;
	float4 depth	: SV_Target2;
};

//======================================================================================================

//
// Vertex Shader
//

PixelInput MaterialGBufferVS(VertexInput input)
{
	PixelInput output;
	
	float3 position = input.position;
	float4x4 world = g_World;
	
	if (g_UsingInstancing == 1)
	{
		if (g_UsingDisplacementMap == 1)
		{
			input.instancePos.y = g_DisplacementMap.SampleLevel(g_PointSampler, input.uv, 0).r;
		}

		output.instanceIndex = (float)input.instanceID;
		
		position.x += input.instancePos.x;
		position.y += input.instancePos.y;
		position.z += input.instancePos.z;
		
		// If the instance has a separate rotation and scale build the new world matrix
		world = CreateWorldMatrix(input.instanceRot, input.instanceScl);
	}
	else
	{
		if (g_UsingDisplacementMap == 1)
		{
			position.y = g_DisplacementMap.SampleLevel(g_PointSampler, input.uv, 0).r;
		}
		
		output.instanceIndex = 0.0f;
	}
	
	output.position = mul(float4(position, 1), world);
	output.position = mul(output.position, g_View);
	output.position = mul(output.position, g_Proj);
	
	output.uv = input.uv;
	
	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal);
	
	output.depth.x = output.position.z;
	output.depth.y = output.position.w;
	
	return output;
}

//======================================================================================================

//
// Pixel Shader
//

PixelOutput MaterialGBufferPS(PixelInput input) : SV_Target
{
	float4 textureColor = float4(0, 0, 0, 1);
    
	// 1 - u: flips the texture horizontally
	if (g_FlipTextureH == 1)
		input.uv.x = 1 - input.uv.x;
	
	// 1 - v: flips the texture vertically
	if (g_FlipTextureV == 1)
		input.uv.y = 1 - input.uv.y;
	
	//============================================ Texturing
	
	float4 finalTextureColor = float4(0, 0, 0, 1);
	float4 diffuseLayers0 = float4(0, 0, 0, 1);
	float4 diffuseLayers1 = float4(0, 0, 0, 1);
	float4 diffuseLayers2 = float4(0, 0, 0, 1);
	float4 diffuseLayers3 = float4(0, 0, 0, 1);
	float4 instanceTexture = float4(0, 0, 0, 1);
	
	if (g_UsingDiffuseArrayTexture == 1)
	{
		if (g_UsingInstancing == 1)
		{
			instanceTexture = g_MaterialDiffuseArrayTexture.Sample( g_LinearSampler, float3(input.uv, input.instanceIndex) );
		}
		else
		{
			// Sample layers in texture array.
			diffuseLayers0 = g_MaterialDiffuseArrayTexture.Sample( g_LinearSampler, float3(input.uv, 0.0f) );
			diffuseLayers1 = g_MaterialDiffuseArrayTexture.Sample( g_LinearSampler, float3(input.uv, 1.0f) );
			diffuseLayers2 = g_MaterialDiffuseArrayTexture.Sample( g_LinearSampler, float3(input.uv, 2.0f) );
			diffuseLayers3 = g_MaterialDiffuseArrayTexture.Sample( g_LinearSampler, float3(input.uv, 3.0f) );
		}
	}
	else
	{
		if (g_UsingDiffuseTexture == 1)
		{
			finalTextureColor = g_MaterialDiffuseTexture.Sample( g_LinearSampler, input.uv );
		}
	}
    
	//============================================ Blend Mapping
	
	if (g_UsingBlendMapTexture == 1)
	{
		if (g_UsingDiffuseArrayTexture == 1)
		{
			// Sample the blend map.
			float4 t = g_MaterialBlendMapTexture.Sample( g_LinearSampler, input.uv );
			
			// Blend the layers on top of each other.
			finalTextureColor = diffuseLayers0;
			finalTextureColor = lerp(finalTextureColor, diffuseLayers1, t.r);
			finalTextureColor = lerp(finalTextureColor, diffuseLayers2, t.g);
			finalTextureColor = lerp(finalTextureColor, diffuseLayers3, t.b);
		}
	}
	else
	{
		if (g_UsingDiffuseArrayTexture == 1)
			finalTextureColor = saturate(diffuseLayers0 + diffuseLayers1 + diffuseLayers2 + diffuseLayers3);
	}
	
	if ((g_UsingInstancing == 1) && (g_UsingDiffuseArrayTexture == 1))
	{
		finalTextureColor += instanceTexture;
	}
	
	// Now add the texture color
	textureColor = finalTextureColor;
	
	//============================================ Normal Mapping
	
	//float depth = input.depth.z / input.depth.w;
	
	float3 normal = input.normal;
	
	if (g_UsingNormalMapTexture == 1)
	{
		float4 normalMap = g_MaterialNormalMapTexture.Sample( g_LinearSampler, input.uv );
		normal = (normalMap.rgb * 2.0f) - 1.0f;
		normal = normalize(normal);
	}
	
	//============================================ Detail Mapping
	
	if (g_UsingDetailMapTexture == 1)
	{
		float4 detailColor = g_MaterialDetailMapTexture.Sample(g_LinearSampler, input.uv);
		textureColor *= detailColor * g_DetailBrightness;
	}

	//============================================ Alpha Map Blending (Alpha-to-Coverage)
	
	if (g_UsingAlphaMapTexture == 1)
	{
		float4 alphaMapColor = g_MaterialAlphaMapTexture.Sample( g_LinearSampler, input.uv );
		if (alphaMapColor.x == g_AlphaToCoverageValue.x && alphaMapColor.y == g_AlphaToCoverageValue.y && alphaMapColor.z == g_AlphaToCoverageValue.z)
		{
			clip(-1);
		}
	}
	
	PixelOutput output;
	
	output.colors = float4(textureColor.rgb, 0.0f);
	output.colors.a = g_MaterialSpecularIntensity;
	output.normals = float4(input.normal, g_MaterialSpecularPower);
	output.depth = float4(input.depth.x / input.depth.y, 0.0, 0.0, 1.0);

	return output;
}

PixelOutput MaterialGBufferWireframePS(PixelInput input) : SV_Target
{
	PixelOutput output;
    
	output.colors = float4(0.9f, 0.9f, 0.9f, 1);
	output.normals = float4(0.5f, 0.5f, 0.5f, 1);
	output.depth = float4(1, 0, 0, 1);
	
	return output;
}